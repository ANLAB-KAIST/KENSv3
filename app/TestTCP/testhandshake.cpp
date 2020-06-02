/*
 * testhandshake.cpp
 *
 *  Created on: 2015. 3. 13.
 *      Author: leeopop
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

class TestHandshake_Accept : public SystemCallApplication, private TCPApplication
{
public:
	TestHandshake_Accept(Host* host, const std::unordered_map<std::string, std::string> &env) : SystemCallApplication(host), TCPApplication(this)
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

		int accept_count = atoi(env["ACCEPT_COUNT"].c_str());
		int expected_accept = atoi(env["EXPECT_ACCEPT"].c_str());
		long accept_period = atoi(env["ACCEPT_PERIOD"].c_str());

		std::vector<int> client_sockets;

		for(int k=0; k<accept_count; k++)
		{
			struct sockaddr_in client_addr;
			socklen_t client_len = sizeof(client_addr);
			memset(&client_addr, 0, client_len);
			int client_fd = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
			if(client_fd >= 0)
			{
				EXPECT_EQ(client_len, sizeof(client_addr));
				EXPECT_EQ(client_addr.sin_family, AF_INET);

				struct sockaddr_in temp_addr;
				socklen_t temp_len = sizeof(temp_addr);
				int ret = getsockname(client_fd, (struct sockaddr*)&temp_addr, &temp_len);
				EXPECT_EQ(ret, 0);
				EXPECT_TRUE( (addr.sin_addr.s_addr == 0) ||
						(addr.sin_addr.s_addr == temp_addr.sin_addr.s_addr));
				EXPECT_EQ(addr.sin_family, temp_addr.sin_family);
				EXPECT_EQ(addr.sin_port, temp_addr.sin_port);

				client_sockets.push_back(client_fd);
			}
			usleep(accept_period);
		}

		EXPECT_EQ((int)client_sockets.size(), expected_accept);
		for(auto client_fd : client_sockets)
		{
			int same_count = 0;
			for(auto client_fd2 : client_sockets)
			{
				if(client_fd == client_fd2)
					same_count++;
			}
			EXPECT_EQ(same_count, 1);
		}

		for(auto client_fd : client_sockets)
		{
			close(client_fd);
		}

		close(server_socket);
	}
};

class TestHandshake_Connect : public SystemCallApplication, private TCPApplication
{
public:
	TestHandshake_Connect(Host* host, const std::unordered_map<std::string, std::string> &env) : SystemCallApplication(host), TCPApplication(this)
	{
		this->env = env;
	}
protected:
	std::unordered_map<std::string, std::string> env;
protected:
	void E_Main()
	{
		std::vector<int> client_sockets;
		std::vector<int> client_ports;
		int connect_count = atoi(env["CONNECT_COUNT"].c_str());
		int expected_connect = atoi(env["EXPECT_CONNECT"].c_str());
		long connect_period = atoi(env["CONNECT_PERIOD"].c_str());

		long connect_time = atol(env["CONNECT_TIME"].c_str());
		usleep(connect_time);

		for(int k=0; k<connect_count; k++)
		{
			int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			struct sockaddr_in addr;
			socklen_t len = sizeof(addr);
			memset(&addr, 0, len);

			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = inet_addr(env["CONNECT_ADDR"].c_str());
			addr.sin_port = htons(atoi(env["CONNECT_PORT"].c_str()));

			int ret = connect(client_socket, (struct sockaddr*)&addr, len);
			if(ret == 0)
			{
				struct sockaddr_in temp_addr;
				socklen_t temp_len = sizeof(temp_addr);
				int ret = getpeername(client_socket, (struct sockaddr*)&temp_addr, &temp_len);
				EXPECT_EQ(ret, 0);
				EXPECT_EQ(addr.sin_addr.s_addr, temp_addr.sin_addr.s_addr);
				EXPECT_EQ(addr.sin_family, temp_addr.sin_family);
				EXPECT_EQ(addr.sin_port, temp_addr.sin_port);

				ret = getsockname(client_socket, (struct sockaddr*)&temp_addr, &temp_len);
				EXPECT_EQ(ret, 0);
				for(int other_port : client_ports)
				{
					EXPECT_NE((int)temp_addr.sin_port, other_port);
				}

				client_sockets.push_back(client_socket);
				client_ports.push_back(temp_addr.sin_port);
			}

			usleep(connect_period);
		}

		EXPECT_EQ((int)client_sockets.size(), expected_connect);
		for(auto client_fd : client_sockets)
		{
			int same_count = 0;
			for(auto client_fd2 : client_sockets)
			{
				if(client_fd == client_fd2)
					same_count++;
			}
			EXPECT_EQ(same_count, 1);
		}

		for(auto client_fd : client_sockets)
		{
			close(client_fd);
		}
	}
};

TEST_F(TestEnv_Reliable, TestAccept_Backlog1)
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
	accept_env["BACKLOG"] = "3";
	accept_env["LISTEN_TIME"] = "0";
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::SEC), TimeUtil::USEC);
	accept_env["ACCEPT_COUNT"] = "10";
	accept_env["EXPECT_ACCEPT"] = "3";
	accept_env["ACCEPT_PERIOD"] = "0";

	connect_env["CONNECT_ADDR"] = host1_ip;
	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_COUNT"] = "1";
	connect_env["EXPECT_CONNECT"] = "1";
	connect_env["CONNECT_PERIOD"] = "0";

	TestHandshake_Accept server(host1, accept_env);

	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::USEC), TimeUtil::USEC);
	TestHandshake_Connect client1(host2, connect_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2,TimeUtil::USEC), TimeUtil::USEC);
	TestHandshake_Connect client2(host2, connect_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(3,TimeUtil::USEC), TimeUtil::USEC);
	TestHandshake_Connect client3(host2, connect_env);

	connect_env["EXPECT_CONNECT"] = "0";

	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(4,TimeUtil::USEC), TimeUtil::USEC);
	TestHandshake_Connect client4(host2, connect_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(5,TimeUtil::USEC), TimeUtil::USEC);
	TestHandshake_Connect client5(host2, connect_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(6,TimeUtil::USEC), TimeUtil::USEC);
	TestHandshake_Connect client6(host2, connect_env);

	server.initialize();
	client1.initialize();
	client2.initialize();
	client3.initialize();
	client4.initialize();
	client5.initialize();
	client6.initialize();

	this->runTest();
}

TEST_F(TestEnv_Reliable, TestAccept_Backlog2)
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
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::SEC), TimeUtil::USEC);
	accept_env["ACCEPT_COUNT"] = "10";
	accept_env["EXPECT_ACCEPT"] = "6";
	accept_env["ACCEPT_PERIOD"] = "0";

	connect_env["CONNECT_ADDR"] = host1_ip;
	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_COUNT"] = "1";
	connect_env["EXPECT_CONNECT"] = "1";
	connect_env["CONNECT_PERIOD"] = "0";


	TestHandshake_Accept server(host1, accept_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(10,TimeUtil::MSEC), TimeUtil::USEC);
	TestHandshake_Connect client1(host2, connect_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(20,TimeUtil::MSEC), TimeUtil::USEC);
	TestHandshake_Connect client2(host2, connect_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(30,TimeUtil::MSEC), TimeUtil::USEC);
	TestHandshake_Connect client3(host2, connect_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(40,TimeUtil::MSEC), TimeUtil::USEC);
	TestHandshake_Connect client4(host2, connect_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(50,TimeUtil::MSEC), TimeUtil::USEC);
	TestHandshake_Connect client5(host2, connect_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(60,TimeUtil::MSEC), TimeUtil::USEC);
	TestHandshake_Connect client6(host2, connect_env);

	server.initialize();
	client1.initialize();
	client2.initialize();
	client3.initialize();
	client4.initialize();
	client5.initialize();
	client6.initialize();

	this->runTest();
}

TEST_F(TestEnv_Any, TestAccept_BeforeAccept)
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
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::SEC), TimeUtil::USEC);
	accept_env["ACCEPT_COUNT"] = "1";
	accept_env["EXPECT_ACCEPT"] = "1";
	accept_env["ACCEPT_PERIOD"] = "0";

	connect_env["CONNECT_ADDR"] = host1_ip;
	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_COUNT"] = "1";
	connect_env["EXPECT_CONNECT"] = "1";
	connect_env["CONNECT_PERIOD"] = "0";


	TestHandshake_Accept server(host1, accept_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(100,TimeUtil::MSEC), TimeUtil::USEC);
	TestHandshake_Connect client1(host2, connect_env);

	server.initialize();
	client1.initialize();

	this->runTest();
}

TEST_F(TestEnv_Any, TestAccept_AfterAccept)
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
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::SEC), TimeUtil::USEC);
	accept_env["ACCEPT_COUNT"] = "1";
	accept_env["EXPECT_ACCEPT"] = "1";
	accept_env["ACCEPT_PERIOD"] = "0";

	connect_env["CONNECT_ADDR"] = host1_ip;
	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_COUNT"] = "1";
	connect_env["EXPECT_CONNECT"] = "1";
	connect_env["CONNECT_PERIOD"] = "0";


	TestHandshake_Accept server(host1, accept_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2,TimeUtil::SEC), TimeUtil::USEC);
	TestHandshake_Connect client1(host2, connect_env);

	server.initialize();
	client1.initialize();

	this->runTest();
}

TEST_F(TestEnv_Any, TestAccept_MultipleInterface1)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t ip1[4];
	uint8_t ip1_2[4];
	uint8_t ip2[4];
	host1->getIPAddr(ip1, 0);
	host1->getIPAddr(ip1_2, 1);
	host2->getIPAddr(ip2, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1], ip1[2], ip1[3]);
	std::string host1_ip(str_buffer);
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1_2[0], ip1_2[1], ip1_2[2], ip1_2[3]);
	std::string host1_ip2(str_buffer);
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1], ip2[2], ip2[3]);
	std::string host2_ip(str_buffer);


	accept_env["LISTEN_PORT"] = "9999";
	accept_env["BACKLOG"] = "1";
	accept_env["LISTEN_TIME"] = "0";
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::SEC), TimeUtil::USEC);
	accept_env["ACCEPT_PERIOD"] = "0";

	accept_env["ACCEPT_COUNT"] = "3";
	accept_env["EXPECT_ACCEPT"] = "3";
	accept_env["LISTEN_ADDR"] = host1_ip;
	TestHandshake_Accept server1(host1, accept_env);

	accept_env["ACCEPT_COUNT"] = "5";
	accept_env["EXPECT_ACCEPT"] = "5";
	accept_env["LISTEN_ADDR"] = host1_ip2;
	TestHandshake_Accept server2(host1, accept_env);

	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_PERIOD"] = "1";
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2,TimeUtil::SEC), TimeUtil::USEC);


	connect_env["CONNECT_ADDR"] = host1_ip;
	connect_env["CONNECT_COUNT"] = "3";
	connect_env["EXPECT_CONNECT"] = "3";
	TestHandshake_Connect client1(host2, connect_env);

	connect_env["CONNECT_ADDR"] = host1_ip2;
	connect_env["CONNECT_COUNT"] = "5";
	connect_env["EXPECT_CONNECT"] = "5";
	TestHandshake_Connect client2(host2, connect_env);

	server1.initialize();
	client1.initialize();
	server2.initialize();
	client2.initialize();

	this->runTest();
}

TEST_F(TestEnv_Any, TestAccept_MultipleInterface2)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t ip1[4];
	uint8_t ip1_2[4];
	uint8_t ip2[4];
	host1->getIPAddr(ip1, 0);
	host1->getIPAddr(ip1_2, 1);
	host2->getIPAddr(ip2, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1], ip1[2], ip1[3]);
	std::string host1_ip(str_buffer);
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1_2[0], ip1_2[1], ip1_2[2], ip1_2[3]);
	std::string host1_ip2(str_buffer);
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1], ip2[2], ip2[3]);
	std::string host2_ip(str_buffer);


	accept_env["LISTEN_PORT"] = "9999";
	accept_env["BACKLOG"] = "1";
	accept_env["LISTEN_TIME"] = "0";
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::SEC), TimeUtil::USEC);
	accept_env["ACCEPT_PERIOD"] = "0";

	accept_env["ACCEPT_COUNT"] = "4";
	accept_env["EXPECT_ACCEPT"] = "4";
	accept_env["LISTEN_ADDR"] = host1_ip;
	TestHandshake_Accept server1(host1, accept_env);

	accept_env["ACCEPT_COUNT"] = "2";
	accept_env["EXPECT_ACCEPT"] = "2";
	accept_env["LISTEN_ADDR"] = host1_ip2;
	TestHandshake_Accept server2(host1, accept_env);

	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_PERIOD"] = "1";
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(100,TimeUtil::MSEC), TimeUtil::USEC);


	connect_env["CONNECT_ADDR"] = host1_ip;
	connect_env["CONNECT_COUNT"] = "4";
	connect_env["EXPECT_CONNECT"] = "4";
	TestHandshake_Connect client1(host2, connect_env);

	connect_env["CONNECT_ADDR"] = host1_ip2;
	connect_env["CONNECT_COUNT"] = "2";
	connect_env["EXPECT_CONNECT"] = "2";
	TestHandshake_Connect client2(host2, connect_env);

	server1.initialize();
	client1.initialize();
	server2.initialize();
	client2.initialize();

	this->runTest();
}

TEST_F(TestEnv_Any, TestConnect_BeforeAccept)
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
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::SEC), TimeUtil::USEC);
	accept_env["ACCEPT_COUNT"] = "1";
	accept_env["EXPECT_ACCEPT"] = "1";
	accept_env["ACCEPT_PERIOD"] = "0";

	connect_env["CONNECT_ADDR"] = host2_ip;
	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_COUNT"] = "1";
	connect_env["EXPECT_CONNECT"] = "1";
	connect_env["CONNECT_PERIOD"] = "0";


	TestHandshake_Accept server(host2, accept_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(100,TimeUtil::MSEC), TimeUtil::USEC);
	TestHandshake_Connect client1(host1, connect_env);

	server.initialize();
	client1.initialize();

	this->runTest();
}

TEST_F(TestEnv_Any, TestConnect_AfterAccept)
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
	accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::SEC), TimeUtil::USEC);
	accept_env["ACCEPT_COUNT"] = "1";
	accept_env["EXPECT_ACCEPT"] = "1";
	accept_env["ACCEPT_PERIOD"] = "0";

	connect_env["CONNECT_ADDR"] = host2_ip;
	connect_env["CONNECT_PORT"] = "9999";
	connect_env["CONNECT_COUNT"] = "1";
	connect_env["EXPECT_CONNECT"] = "1";
	connect_env["CONNECT_PERIOD"] = "0";


	TestHandshake_Accept server(host2, accept_env);
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2,TimeUtil::SEC), TimeUtil::USEC);
	TestHandshake_Connect client1(host1, connect_env);

	server.initialize();
	client1.initialize();

	this->runTest();
}

class TestHandshake_SimultaneousConnect : public SystemCallApplication, private TCPApplication
{
public:
	TestHandshake_SimultaneousConnect(Host* host, const std::unordered_map<std::string, std::string> &env) : SystemCallApplication(host), TCPApplication(this)
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

		struct sockaddr_in bind_addr;
		socklen_t bind_len = sizeof(bind_addr);
		memset(&bind_addr, 0, bind_len);

		bind_addr.sin_family = AF_INET;
		bind_addr.sin_addr.s_addr = inet_addr(env["BIND_ADDR"].c_str());
		bind_addr.sin_port = htons(atoi(env["BIND_PORT"].c_str()));

		int ret = bind(client_socket, (struct sockaddr*)&bind_addr, bind_len);
		EXPECT_EQ(ret, 0);

		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		memset(&addr, 0, len);

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(env["CONNECT_ADDR"].c_str());
		addr.sin_port = htons(atoi(env["CONNECT_PORT"].c_str()));

		ret = connect(client_socket, (struct sockaddr*)&addr, len);
		EXPECT_EQ(ret, 0);

		struct sockaddr_in temp_addr;
		socklen_t temp_len = sizeof(temp_addr);
		ret = getpeername(client_socket, (struct sockaddr*)&temp_addr, &temp_len);
		EXPECT_EQ(ret, 0);
		EXPECT_EQ(addr.sin_addr.s_addr, temp_addr.sin_addr.s_addr);
		EXPECT_EQ(addr.sin_family, temp_addr.sin_family);
		EXPECT_EQ(addr.sin_port, temp_addr.sin_port);

		ret = getsockname(client_socket, (struct sockaddr*)&temp_addr, &temp_len);
		EXPECT_EQ(ret, 0);

		EXPECT_EQ(bind_addr.sin_addr.s_addr, temp_addr.sin_addr.s_addr);
		EXPECT_EQ(bind_addr.sin_family, temp_addr.sin_family);
		EXPECT_EQ(bind_addr.sin_port, temp_addr.sin_port);

		close(client_socket);
	}
};

TEST_F(TestEnv_Any, TestConnect_SimultaneousConnect)
{
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

	connect_env["CONNECT_ADDR"] = host2_ip;
	connect_env["CONNECT_PORT"] = "12345";
	connect_env["BIND_ADDR"] = host1_ip;
	connect_env["BIND_PORT"] = "22222";


	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::SEC), TimeUtil::USEC);
	TestHandshake_SimultaneousConnect client1(host1, connect_env);


	connect_env["CONNECT_ADDR"] = host1_ip;
	connect_env["CONNECT_PORT"] = "22222";
	connect_env["BIND_ADDR"] = host2_ip;
	connect_env["BIND_PORT"] = "12345";
	connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1,TimeUtil::SEC), TimeUtil::USEC);
	TestHandshake_SimultaneousConnect client2(host2, connect_env);

	client1.initialize();
	client2.initialize();

	this->runTest();
}
