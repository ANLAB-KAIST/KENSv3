/*
 * testcongestion.cpp
 *
 *  Created on: 2015. 3. 16.
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

extern "C"
{
#include <stdlib.h>
#include <time.h>
}

using namespace E;

class TestCongestion_Accept : public SystemCallApplication, private TCPApplication
{
public:
	TestCongestion_Accept(Host* host, const std::unordered_map<std::string, std::string> &env) : SystemCallApplication(host), TCPApplication(this)
{
		this->env = env;
}
protected:
	std::unordered_map<std::string, std::string> env;
protected:
	void E_Main()
	{
		int connection_timeout = atoi(env["CONNECTION_TIMEOUT"].c_str());
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

		long start_time = atol(env["START_TIME"].c_str());

		struct timeval tv;
		ret = gettimeofday(&tv, 0);
		EXPECT_EQ(ret, 0);

		long sleep_time = start_time - (1000*1000*tv.tv_sec) - tv.tv_usec;
		EXPECT_GE(sleep_time, 0);
		//printf("connect sleep: %ld\n", sleep_time);
		usleep(sleep_time);

		unsigned int seed = atoi(env["RANDOM_SEED"].c_str());
		int is_send = atoi(env["SENDER"].c_str());
		int buffer_size = atoi(env["BUFFER_SIZE"].c_str());
		int loop_count = atoi(env["LOOP_COUNT"].c_str());
		long expect_size = atoi(env["EXPECT_SIZE"].c_str());

		uint8_t* send_buffer = (uint8_t*)malloc(buffer_size);
		uint8_t* recv_buffer = (uint8_t*)malloc(buffer_size);

		int stop = 0;
		int loop = 0;
		long total_size = 0;
		while(!stop)
		{
			for(int k=0; k<buffer_size; k++)
				send_buffer[k] = rand_r(&seed) & 0xFF;

			if(is_send)
			{
				int remaining = buffer_size;
				int write_byte = 0;
				while((write_byte = write(client_fd, send_buffer + (buffer_size - remaining), remaining)) >= 0)
				{
					total_size += write_byte;
					remaining -= write_byte;
					EXPECT_GE(remaining, 0);
					if(remaining == 0)
						break;
				}
				if(write_byte < 0)
					break;
			}
			else
			{
				int remaining = buffer_size;
				int read_byte = 0;
				while((read_byte = read(client_fd, recv_buffer + (buffer_size - remaining), remaining)) >= 0)
				{
					total_size += read_byte;
					remaining -= read_byte;
					EXPECT_GE(remaining, 0);
					if(remaining == 0)
						break;
				}
				if(buffer_size - remaining > 0)
				{
					for(int j=0; j<buffer_size - remaining; j++)
					{
						EXPECT_EQ(send_buffer[j], recv_buffer[j]);
					}
				}
				if(read_byte < 0)
					break;
			}

			loop++;
			if(loop_count != 0 && loop_count <= loop)
				break;
		}

		free(send_buffer);
		free(recv_buffer);

		EXPECT_EQ(expect_size, total_size);
		struct timeval timeval;
		gettimeofday(&timeval, 0);
		EXPECT_LT(timeval.tv_sec, connection_timeout);

		close(client_fd);
		close(server_socket);
	}
};

class TestCongestion_Connect : public SystemCallApplication, private TCPApplication
{
public:
	TestCongestion_Connect(Host* host, const std::unordered_map<std::string, std::string> &env) : SystemCallApplication(host), TCPApplication(this)
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

		long start_time = atol(env["START_TIME"].c_str());

		struct timeval tv;
		ret = gettimeofday(&tv, 0);
		EXPECT_EQ(ret, 0);

		long sleep_time = start_time - (1000*1000*tv.tv_sec) - tv.tv_usec;
		EXPECT_GE(sleep_time, 0);
		//printf("connect sleep: %ld\n", sleep_time);
		usleep(sleep_time);

		unsigned int seed = atoi(env["RANDOM_SEED"].c_str());
		int is_send = atoi(env["SENDER"].c_str());
		int buffer_size = atoi(env["BUFFER_SIZE"].c_str());
		int loop_count = atoi(env["LOOP_COUNT"].c_str());
		long expect_size = atoi(env["EXPECT_SIZE"].c_str());

		uint8_t* send_buffer = (uint8_t*)malloc(buffer_size);
		uint8_t* recv_buffer = (uint8_t*)malloc(buffer_size);

		int stop = 0;
		int loop = 0;
		long total_size = 0;
		while(!stop)
		{
			for(int k=0; k<buffer_size; k++)
				send_buffer[k] = rand_r(&seed) & 0xFF;

			if(is_send)
			{
				int remaining = buffer_size;
				int write_byte = 0;
				while((write_byte = write(client_socket, send_buffer + (buffer_size - remaining), remaining)) >= 0)
				{
					total_size += write_byte;
					remaining -= write_byte;
					EXPECT_GE(remaining, 0);
					if(remaining == 0)
						break;
				}
				if(write_byte < 0)
					break;
			}
			else
			{
				int remaining = buffer_size;
				int read_byte = 0;
				while((read_byte = read(client_socket, recv_buffer + (buffer_size - remaining), remaining)) >= 0)
				{
					total_size += read_byte;
					remaining -= read_byte;
					EXPECT_GE(remaining, 0);
					if(remaining == 0)
						break;
				}
				if(buffer_size - remaining > 0)
				{
					for(int j=0; j<buffer_size - remaining; j++)
					{
						EXPECT_EQ(send_buffer[j], recv_buffer[j]);
					}
				}
				if(read_byte < 0)
					break;
			}

			loop++;
			if(loop_count != 0 && loop_count <= loop)
				break;
		}

		free(send_buffer);
		free(recv_buffer);

		EXPECT_EQ(expect_size, total_size);

		close(client_socket);
	}
};

TEST_F(TestEnv_Congestion0, TestCongestion0)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t server_ip[4];
	server_host->getIPAddr(server_ip, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", server_ip[0], server_ip[1], server_ip[2], server_ip[3]);
	std::string connect_addr(str_buffer);

	TestCongestion_Connect** clients = new TestCongestion_Connect*[num_client];
	TestCongestion_Accept** servers = new TestCongestion_Accept*[num_client];

	for(int k=0; k<num_client; k++)
	{
		snprintf(str_buffer, sizeof(str_buffer), "%d", k+10000);
		std::string connect_port(str_buffer);

		Time start_time = TimeUtil::makeTime(1,TimeUtil::SEC);
		start_time += TimeUtil::makeTime(0,TimeUtil::SEC);

		accept_env["RANDOM_SEED"] = "104729";
		accept_env["LISTEN_ADDR"] = "0.0.0.0";
		accept_env["LISTEN_PORT"] = connect_port;
		accept_env["BACKLOG"] = "1";
		accept_env["LISTEN_TIME"] = "0";
		accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1000,TimeUtil::USEC), TimeUtil::USEC);
		accept_env["START_TIME"] = TimeUtil::printTime(start_time, TimeUtil::USEC);

		connect_env["RANDOM_SEED"] = "104729";
		connect_env["CONNECT_PORT"] = connect_port;
		connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2000,TimeUtil::USEC), TimeUtil::USEC);
		connect_env["START_TIME"] = TimeUtil::printTime(start_time, TimeUtil::USEC);

		connect_env["CONNECT_ADDR"] = connect_addr;
		connect_env["BUFFER_SIZE"] = "1024";
		connect_env["LOOP_COUNT"] = "100000";
		connect_env["SENDER"] = "1";
		connect_env["EXPECT_SIZE"] = "102400000";
		clients[k] = new TestCongestion_Connect(client_hosts[k], connect_env);

		accept_env["SENDER"] = "0";
		accept_env["BUFFER_SIZE"] = "1024";
		accept_env["LOOP_COUNT"] = "0";
		accept_env["EXPECT_SIZE"] = "102400000";
		accept_env["CONNECTION_TIMEOUT"] = "92";
		servers[k] = new TestCongestion_Accept(server_host, accept_env);

		clients[k]->initialize();
		servers[k]->initialize();
	}

	this->runTest();

	for(int k=0; k<num_client; k++)
	{
		delete servers[k];
		delete clients[k];
	}

	delete[] servers;
	delete[] clients;
}

TEST_F(TestEnv_Congestion1, TestCongestion1)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t server_ip[4];
	server_host->getIPAddr(server_ip, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", server_ip[0], server_ip[1], server_ip[2], server_ip[3]);
	std::string connect_addr(str_buffer);

	TestCongestion_Connect** clients = new TestCongestion_Connect*[num_client];
	TestCongestion_Accept** servers = new TestCongestion_Accept*[num_client];

	for(int k=0; k<num_client; k++)
	{
		snprintf(str_buffer, sizeof(str_buffer), "%d", k+10000);
		std::string connect_port(str_buffer);

		Time start_time = TimeUtil::makeTime(1,TimeUtil::SEC);
		start_time += TimeUtil::makeTime(0,TimeUtil::SEC);

		accept_env["RANDOM_SEED"] = "104729";
		accept_env["LISTEN_ADDR"] = "0.0.0.0";
		accept_env["LISTEN_PORT"] = connect_port;
		accept_env["BACKLOG"] = "1";
		accept_env["LISTEN_TIME"] = "0";
		accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1000,TimeUtil::USEC), TimeUtil::USEC);
		accept_env["START_TIME"] = TimeUtil::printTime(start_time, TimeUtil::USEC);

		connect_env["RANDOM_SEED"] = "104729";
		connect_env["CONNECT_PORT"] = connect_port;
		connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2000,TimeUtil::USEC), TimeUtil::USEC);
		connect_env["START_TIME"] = TimeUtil::printTime(start_time, TimeUtil::USEC);

		connect_env["CONNECT_ADDR"] = connect_addr;
		connect_env["BUFFER_SIZE"] = "1024";
		connect_env["LOOP_COUNT"] = "10000";
		connect_env["SENDER"] = "1";
		connect_env["EXPECT_SIZE"] = "10240000";
		clients[k] = new TestCongestion_Connect(client_hosts[k], connect_env);

		accept_env["SENDER"] = "0";
		accept_env["BUFFER_SIZE"] = "1024";
		accept_env["LOOP_COUNT"] = "0";
		accept_env["EXPECT_SIZE"] = "10240000";
		accept_env["CONNECTION_TIMEOUT"] = "60";
		servers[k] = new TestCongestion_Accept(server_host, accept_env);

		clients[k]->initialize();
		servers[k]->initialize();
	}

	this->runTest();

	for(int k=0; k<num_client; k++)
	{
		delete servers[k];
		delete clients[k];
	}

	delete[] servers;
	delete[] clients;
}

TEST_F(TestEnv_Congestion2, TestCongestion2)
{
	std::unordered_map<std::string, std::string> accept_env;
	std::unordered_map<std::string, std::string> connect_env;

	uint8_t server_ip[4];
	server_host->getIPAddr(server_ip, 0);

	char str_buffer[128];
	snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", server_ip[0], server_ip[1], server_ip[2], server_ip[3]);
	std::string connect_addr(str_buffer);

	TestCongestion_Connect** clients = new TestCongestion_Connect*[num_client];
	TestCongestion_Accept** servers = new TestCongestion_Accept*[num_client];

	for(int k=0; k<num_client; k++)
	{
		snprintf(str_buffer, sizeof(str_buffer), "%d", k+10000);
		std::string connect_port(str_buffer);

		Time start_time = TimeUtil::makeTime(1,TimeUtil::SEC);
		start_time += TimeUtil::makeTime(0,TimeUtil::SEC);

		accept_env["RANDOM_SEED"] = "104729";
		accept_env["LISTEN_ADDR"] = "0.0.0.0";
		accept_env["LISTEN_PORT"] = connect_port;
		accept_env["BACKLOG"] = "1";
		accept_env["LISTEN_TIME"] = "0";
		accept_env["ACCEPT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(1000,TimeUtil::USEC), TimeUtil::USEC);
		accept_env["START_TIME"] = TimeUtil::printTime(start_time, TimeUtil::USEC);

		connect_env["RANDOM_SEED"] = "104729";
		connect_env["CONNECT_PORT"] = connect_port;
		connect_env["CONNECT_TIME"] = TimeUtil::printTime(TimeUtil::makeTime(2000,TimeUtil::USEC), TimeUtil::USEC);
		connect_env["START_TIME"] = TimeUtil::printTime(start_time, TimeUtil::USEC);

		connect_env["CONNECT_ADDR"] = connect_addr;
		connect_env["BUFFER_SIZE"] = "1024";
		connect_env["LOOP_COUNT"] = "10000";
		connect_env["SENDER"] = "1";
		connect_env["EXPECT_SIZE"] = "10240000";
		clients[k] = new TestCongestion_Connect(client_hosts[k], connect_env);

		accept_env["SENDER"] = "0";
		accept_env["BUFFER_SIZE"] = "1024";
		accept_env["LOOP_COUNT"] = "0";
		accept_env["EXPECT_SIZE"] = "10240000";
		accept_env["CONNECTION_TIMEOUT"] = "150";
		servers[k] = new TestCongestion_Accept(server_host, accept_env);

		clients[k]->initialize();
		servers[k]->initialize();
	}

	this->runTest();

	for(int k=0; k<num_client; k++)
	{
		delete servers[k];
		delete clients[k];
	}

	delete[] servers;
	delete[] clients;
}
