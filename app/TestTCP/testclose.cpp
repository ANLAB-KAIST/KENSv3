/*
 * testclose.cpp
 *
 *  Created on: 2015. 3. 15.
 *      Author: Keunhong Lee
 */


#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Hub.hpp>
#include <E/Networking/Ethernet/E_Ethernet.hpp>
#include <E/Networking/IPv4/E_IPv4.hpp>
#include <E/Networking/TCP/E_TCPApplication.hpp>
#include <E/Networking/TCP/E_TCPSolution.hpp>
#include <E/E_TimeUtil.hpp>

#include <arpa/inet.h>

#include <gtest/gtest.h>
#include "testenv.hpp"

using namespace E;

class TestClose_Accept : public SystemCallApplication, private TCPApplication
{
public:
	TestClose_Accept(Host* host, const std::unordered_map<std::string, std::string> &env) : SystemCallApplication(host), TCPApplication(this)
	{
		this->env = env;
	}
protected:
	std::unordered_map<std::string, std::string> env;
protected:
	void E_Main()
	{
		int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		memset(&addr, 0, len);

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(env["LISTEN_ADDR"].c_str());
		addr.sin_port = htons(atoi(env["LISTEN_PORT"].c_str()));

		int ret = bind(server_socket, (struct sockaddr*)&addr, len);
		EXPECT_EQ(ret, 0);

		long listen_time = atol(env["LISTEN_TIME"].c_str());
		usleep(listen_time);

		ret = listen(server_socket, atoi(env["BACKLOG"].c_str()));
		EXPECT_EQ(ret, 0);

		long accept_time = atol(env["ACCEPT_TIME"].c_str());
		usleep(accept_time);

		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		memset(&client_addr, 0, client_len);
		int client_fd = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
		EXPECT_GE(client_fd, 0);

		EXPECT_EQ(client_len, sizeof(client_addr));
		EXPECT_EQ(client_addr.sin_family, AF_INET);

		struct sockaddr_in temp_addr;
		socklen_t temp_len = sizeof(temp_addr);
		ret = getsockname(client_fd, (struct sockaddr*)&temp_addr, &temp_len);
		EXPECT_EQ(ret, 0);
		EXPECT_TRUE( (addr.sin_addr.s_addr == 0) ||
				(addr.sin_addr.s_addr == temp_addr.sin_addr.s_addr));
		EXPECT_EQ(addr.sin_family, temp_addr.sin_family);
		EXPECT_EQ(addr.sin_port, temp_addr.sin_port);


		long close_time = atol(env["CLOSE_TIME"].c_str());

		struct timeval tv;
		ret = gettimeofday(&tv, 0);
		EXPECT_EQ(ret, 0);

		long sleep_time = close_time - (1000*1000*tv.tv_sec) - tv.tv_usec;
		EXPECT_GE(sleep_time, 0);
		//printf("accept sleep: %ld\n", sleep_time);

		usleep(sleep_time);
		close(client_fd);

		close(server_socket);
	}
};

class TestClose_Connect : public SystemCallApplication, private TCPApplication
{
public:
	TestClose_Connect(Host* host, const std::unordered_map<std::string, std::string> &env) : SystemCallApplication(host), TCPApplication(this)
	{
		this->env = env;
	}
protected:
	std::unordered_map<std::string, std::string> env;
protected:
	void E_Main()
	{

		long connect_time = atol(env["CONNECT_TIME"].c_str());
		usleep(connect_time);

		int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		memset(&addr, 0, len);

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(env["CONNECT_ADDR"].c_str());
		addr.sin_port = htons(atoi(env["CONNECT_PORT"].c_str()));

		int ret = connect(client_socket, (struct sockaddr*)&addr, len);
		EXPECT_GE(ret, 0);

		struct sockaddr_in temp_addr;
		socklen_t temp_len = sizeof(temp_addr);
		ret = getpeername(client_socket, (struct sockaddr*)&temp_addr, &temp_len);
		EXPECT_EQ(ret, 0);
		EXPECT_EQ(addr.sin_addr.s_addr, temp_addr.sin_addr.s_addr);
		EXPECT_EQ(addr.sin_family, temp_addr.sin_family);
		EXPECT_EQ(addr.sin_port, temp_addr.sin_port);

		long close_time = atol(env["CLOSE_TIME"].c_str());

		struct timeval tv;
		ret = gettimeofday(&tv, 0);
		EXPECT_EQ(ret, 0);

		long sleep_time = close_time - (1000*1000*tv.tv_sec) - tv.tv_usec;
		EXPECT_GE(sleep_time, 0);
		//printf("connect sleep: %ld\n", sleep_time);
		usleep(sleep_time);

		close(client_socket);
	}
};


TEST_F(TestEnv_Any, TestClose_Connect_CloseFirst)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t ip1[4];
	uint8_t ip2[4];
	host1->getIPAddr(ip1, 0);
	host2->getIPAddr(ip2, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1], ip1[2], ip1[3]);
	std::string host1_ip(str_buffer);
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1], ip2[2], ip2[3]);
	std::string host2_ip(str_buffer);

	accept_env["LISTEN_ADDR"] = "0.0.0.0";
	accept_env["LISTEN_PORT"] = "9999";
	accept_env["BACKLOG"] = "1";
	accept_env["LISTEN_TIME"] = "0";
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1000,TimeUtil::USEC), TimeUtil::USEC);
	accept_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1020000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2000,TimeUtil::USEC), TimeUtil::USEC);
	connect_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1010000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_ADDR"] = host2_ip;
	TestClose_Connect client(host1, connect_env);
	TestClose_Accept server(host2, accept_env);

	server.initialize();
	client.initialize();

	this->runTest();
}

TEST_F(TestEnv_Any, TestClose_Connect_CloseLater)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t ip1[4];
	uint8_t ip2[4];
	host1->getIPAddr(ip1, 0);
	host2->getIPAddr(ip2, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1], ip1[2], ip1[3]);
	std::string host1_ip(str_buffer);
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1], ip2[2], ip2[3]);
	std::string host2_ip(str_buffer);

	accept_env["LISTEN_ADDR"] = "0.0.0.0";
	accept_env["LISTEN_PORT"] = "9999";
	accept_env["BACKLOG"] = "1";
	accept_env["LISTEN_TIME"] = "0";
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1000,TimeUtil::USEC), TimeUtil::USEC);
	accept_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1010000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2000,TimeUtil::USEC), TimeUtil::USEC);
	connect_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1020000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_ADDR"] = host2_ip;
	TestClose_Connect client(host1, connect_env);
	TestClose_Accept server(host2, accept_env);

	server.initialize();
	client.initialize();

	this->runTest();
}

TEST_F(TestEnv_Any, TestClose_Connect_CloseSimultaneous)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t ip1[4];
	uint8_t ip2[4];
	host1->getIPAddr(ip1, 0);
	host2->getIPAddr(ip2, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1], ip1[2], ip1[3]);
	std::string host1_ip(str_buffer);
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1], ip2[2], ip2[3]);
	std::string host2_ip(str_buffer);

	accept_env["LISTEN_ADDR"] = "0.0.0.0";
	accept_env["LISTEN_PORT"] = "9999";
	accept_env["BACKLOG"] = "1";
	accept_env["LISTEN_TIME"] = "0";
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1000,TimeUtil::USEC), TimeUtil::USEC);
	accept_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1010000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2000,TimeUtil::USEC), TimeUtil::USEC);
	connect_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1010000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_ADDR"] = host2_ip;
	TestClose_Connect client(host1, connect_env);
	TestClose_Accept server(host2, accept_env);

	server.initialize();
	client.initialize();

	this->runTest();
}


//-------

TEST_F(TestEnv_Any, TestClose_Accept_CloseLater)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t ip1[4];
	uint8_t ip2[4];
	host1->getIPAddr(ip1, 0);
	host2->getIPAddr(ip2, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1], ip1[2], ip1[3]);
	std::string host1_ip(str_buffer);
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1], ip2[2], ip2[3]);
	std::string host2_ip(str_buffer);

	accept_env["LISTEN_ADDR"] = "0.0.0.0";
	accept_env["LISTEN_PORT"] = "9999";
	accept_env["BACKLOG"] = "1";
	accept_env["LISTEN_TIME"] = "0";
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1000,TimeUtil::USEC), TimeUtil::USEC);
	accept_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1020000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2000,TimeUtil::USEC), TimeUtil::USEC);
	connect_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1010000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_ADDR"] = host1_ip;
	TestClose_Connect client(host2, connect_env);
	TestClose_Accept server(host1, accept_env);

	server.initialize();
	client.initialize();

	this->runTest();
}

TEST_F(TestEnv_Any, TestClose_Accept_CloseFirst)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t ip1[4];
	uint8_t ip2[4];
	host1->getIPAddr(ip1, 0);
	host2->getIPAddr(ip2, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1], ip1[2], ip1[3]);
	std::string host1_ip(str_buffer);
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1], ip2[2], ip2[3]);
	std::string host2_ip(str_buffer);

	accept_env["LISTEN_ADDR"] = "0.0.0.0";
	accept_env["LISTEN_PORT"] = "9999";
	accept_env["BACKLOG"] = "1";
	accept_env["LISTEN_TIME"] = "0";
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1000,TimeUtil::USEC), TimeUtil::USEC);
	accept_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1010000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2000,TimeUtil::USEC), TimeUtil::USEC);
	connect_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1020000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_ADDR"] = host1_ip;
	TestClose_Connect client(host2, connect_env);
	TestClose_Accept server(host1, accept_env);

	server.initialize();
	client.initialize();

	this->runTest();
}

TEST_F(TestEnv_Any, TestClose_Accept_CloseSimultaneous)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t ip1[4];
	uint8_t ip2[4];
	host1->getIPAddr(ip1, 0);
	host2->getIPAddr(ip2, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1], ip1[2], ip1[3]);
	std::string host1_ip(str_buffer);
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1], ip2[2], ip2[3]);
	std::string host2_ip(str_buffer);

	accept_env["LISTEN_ADDR"] = "0.0.0.0";
	accept_env["LISTEN_PORT"] = "9999";
	accept_env["BACKLOG"] = "1";
	accept_env["LISTEN_TIME"] = "0";
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1000,TimeUtil::USEC), TimeUtil::USEC);
	accept_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1010000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2000,TimeUtil::USEC), TimeUtil::USEC);
	connect_env["CLOSE_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1010000,TimeUtil::USEC), TimeUtil::USEC);

	connect_env["CONNECT_ADDR"] = host1_ip;
	TestClose_Connect client(host2, connect_env);
	TestClose_Accept server(host1, accept_env);

	server.initialize();
	client.initialize();

	this->runTest();
}
