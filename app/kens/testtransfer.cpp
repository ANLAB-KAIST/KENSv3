/*
 * testtransfer.cpp
 *
 *  Created on: 2015. 3. 15.
 *      Author: Keunhong Lee
 */

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/E_TimeUtil.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Hub.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/Ethernet/E_Ethernet.hpp>
#include <E/Networking/IPv4/E_IPv4.hpp>
#include <E/Networking/TCP/E_TCPApplication.hpp>
#include <E/Networking/TCP/E_TCPSolution.hpp>

#include <arpa/inet.h>

#include "testenv.hpp"
#include <gtest/gtest.h>

extern "C" {
#include <stdlib.h>
}

using namespace E;

/**
 * @brief Server application for the transfer test
 *
 * This application is configured by the following environment variables.
 *
 * * `CONNECT_ADDR`, `CONNECT_PORT`: Parameters to configure a listening socket.
 * * `CONNECT_TIME`: Arbitrary sleeping time before starting the handshake. If
 *   the handshake is completed but the established connection is not consumed
 *   by the server's `listen` system call, the backlog count will be decreased
 *   by one.
 * * `START_TIME`: Arbitrary sleeping time before starting the data transfer.
 * * `RANDOM_SEED`: This random seed is used to generate a pseudo-random byte
 *   stream. If a pair of server and client shares the same seed, the seed is
 *   used to generate and verify the data integrity.
 * * `SENDER`: Determines who will send the data. Data transfer can be done in
 *   either client-to-server and server-to-client directions.
 * * `BUFFER_SIZE`: Size of the buffer to be used in every `write` or `read`
 *   system call.
 * * `LOOP_COUNT`: How many times the `write` system call will be invoked.
 * * `EXPECT_SIZE`: Expected length of the received data.
 */
class TestTransfer_Accept : public TCPApplication {
public:
  TestTransfer_Accept(Host &host,
                      const std::unordered_map<std::string, std::string> &env)
      : TCPApplication(host) {
    this->env = env;
  }

protected:
  std::unordered_map<std::string, std::string> env;

protected:
  int E_Main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(env["LISTEN_ADDR"].c_str());
    addr.sin_port = htons(atoi(env["LISTEN_PORT"].c_str()));

    int ret = bind(server_socket, (struct sockaddr *)&addr, len);
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
    int client_fd =
        accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    EXPECT_GE(client_fd, 0);

    EXPECT_EQ(client_len, sizeof(client_addr));
    EXPECT_EQ(client_addr.sin_family, AF_INET);

    struct sockaddr_in temp_addr;
    socklen_t temp_len = sizeof(temp_addr);
    ret = getsockname(client_fd, (struct sockaddr *)&temp_addr, &temp_len);
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE((addr.sin_addr.s_addr == 0) ||
                (addr.sin_addr.s_addr == temp_addr.sin_addr.s_addr));
    EXPECT_EQ(addr.sin_family, temp_addr.sin_family);
    EXPECT_EQ(addr.sin_port, temp_addr.sin_port);

    long start_time = atol(env["START_TIME"].c_str());

    struct timeval tv;
    ret = gettimeofday(&tv, 0);
    EXPECT_EQ(ret, 0);

    long sleep_time = start_time - (1000 * 1000 * tv.tv_sec) - tv.tv_usec;
    EXPECT_GE(sleep_time, 0);
    // printf("connect sleep: %ld\n", sleep_time);
    usleep(sleep_time);

    unsigned int seed = atoi(env["RANDOM_SEED"].c_str());
    int is_send = atoi(env["SENDER"].c_str());
    int buffer_size = atoi(env["BUFFER_SIZE"].c_str());
    int loop_count = atoi(env["LOOP_COUNT"].c_str());
    long expect_size = atoi(env["EXPECT_SIZE"].c_str());

    std::default_random_engine rand_e(seed);
    std::uniform_int_distribution<int> rand_d(0xFF);

    uint8_t *send_buffer = (uint8_t *)malloc(buffer_size);
    uint8_t *recv_buffer = (uint8_t *)malloc(buffer_size);

    int stop = 0;
    int loop = 0;
    long total_size = 0;
    while (!stop) {
      for (int k = 0; k < buffer_size; k++)
        send_buffer[k] = rand_d(rand_e);

      if (is_send) {
        int remaining = buffer_size;
        int write_byte = 0;
        while ((write_byte =
                    write(client_fd, send_buffer + (buffer_size - remaining),
                          remaining)) >= 0) {
          total_size += write_byte;
          remaining -= write_byte;
          EXPECT_GE(remaining, 0);
          if (remaining == 0)
            break;
        }
        if (write_byte < 0)
          break;
      } else {
        int remaining = buffer_size;
        int read_byte = 0;
        while ((read_byte =
                    read(client_fd, recv_buffer + (buffer_size - remaining),
                         remaining)) >= 0) {
          total_size += read_byte;
          remaining -= read_byte;
          EXPECT_GE(remaining, 0);
          if (remaining == 0)
            break;
        }
        if (buffer_size - remaining > 0) {
          for (int j = 0; j < buffer_size - remaining; j++) {
            EXPECT_EQ(send_buffer[j], recv_buffer[j]);
          }
        }
        if (read_byte < 0)
          break;
      }

      loop++;
      if (loop_count != 0 && loop_count <= loop)
        break;
    }

    free(send_buffer);
    free(recv_buffer);

    EXPECT_EQ(expect_size, total_size);

    close(client_fd);
    close(server_socket);
    return 0;
  }
};

/**
 * @brief Client application for the transfer test
 *
 * This application is configured by the following environment variables.
 *
 * * `LISTEN_ADDR`, `LISTEN_PORT`, `BACKLOG`: Parameters to configure a
 *   listening socket.
 * * `ACCEPT_TIME`: Arbitrary sleeping time before accepting sockets. If
 *   connections arrive during the sleep, they will consume the backlog count.
 * * `START_TIME`: Arbitrary sleeping time before starting the data transfer.
 * * `RANDOM_SEED`: This random seed is used to generate a pseudo-random byte
 *   stream. If a pair of server and client shares the same seed, the seed is
 *   used to generate and verify the data integrity.
 * * `SENDER`: Determines who will send the data. Data transfer can be done in
 *   either client-to-server and server-to-client directions.
 * * `BUFFER_SIZE`: Size of the buffer to be used in every `write` or `read`
 *   system call.
 * * `LOOP_COUNT`: How many times the `write` system call will be invoked.
 * * `EXPECT_SIZE`: Expected length of the received data.
 */
class TestTransfer_Connect : public TCPApplication {
public:
  TestTransfer_Connect(Host &host,
                       const std::unordered_map<std::string, std::string> &env)
      : TCPApplication(host) {
    this->env = env;
  }

protected:
  std::unordered_map<std::string, std::string> env;

protected:
  int E_Main() {
    long connect_time = atol(env["CONNECT_TIME"].c_str());
    usleep(connect_time);

    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(env["CONNECT_ADDR"].c_str());
    addr.sin_port = htons(atoi(env["CONNECT_PORT"].c_str()));

    int ret = connect(client_socket, (struct sockaddr *)&addr, len);
    EXPECT_GE(ret, 0);

    struct sockaddr_in temp_addr;
    socklen_t temp_len = sizeof(temp_addr);
    ret = getpeername(client_socket, (struct sockaddr *)&temp_addr, &temp_len);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(addr.sin_addr.s_addr, temp_addr.sin_addr.s_addr);
    EXPECT_EQ(addr.sin_family, temp_addr.sin_family);
    EXPECT_EQ(addr.sin_port, temp_addr.sin_port);

    long start_time = atol(env["START_TIME"].c_str());

    struct timeval tv;
    ret = gettimeofday(&tv, 0);
    EXPECT_EQ(ret, 0);

    long sleep_time = start_time - (1000 * 1000 * tv.tv_sec) - tv.tv_usec;
    EXPECT_GE(sleep_time, 0);
    // printf("connect sleep: %ld\n", sleep_time);
    usleep(sleep_time);

    unsigned int seed = atoi(env["RANDOM_SEED"].c_str());
    int is_send = atoi(env["SENDER"].c_str());
    int buffer_size = atoi(env["BUFFER_SIZE"].c_str());
    int loop_count = atoi(env["LOOP_COUNT"].c_str());
    long expect_size = atoi(env["EXPECT_SIZE"].c_str());

    std::default_random_engine rand_e(seed);
    std::uniform_int_distribution<int> rand_d(0xFF);

    uint8_t *send_buffer = (uint8_t *)malloc(buffer_size);
    uint8_t *recv_buffer = (uint8_t *)malloc(buffer_size);

    int stop = 0;
    int loop = 0;
    long total_size = 0;
    while (!stop) {
      for (int k = 0; k < buffer_size; k++)
        send_buffer[k] = rand_d(rand_e);

      if (is_send) {
        int remaining = buffer_size;
        int write_byte = 0;
        while ((write_byte = write(client_socket,
                                   send_buffer + (buffer_size - remaining),
                                   remaining)) >= 0) {
          total_size += write_byte;
          remaining -= write_byte;
          EXPECT_GE(remaining, 0);
          if (remaining == 0)
            break;
        }
        if (write_byte < 0)
          break;
      } else {
        int remaining = buffer_size;
        int read_byte = 0;
        while ((read_byte =
                    read(client_socket, recv_buffer + (buffer_size - remaining),
                         remaining)) >= 0) {
          total_size += read_byte;
          remaining -= read_byte;
          EXPECT_GE(remaining, 0);
          if (remaining == 0)
            break;
        }
        if (buffer_size - remaining > 0) {
          for (int j = 0; j < buffer_size - remaining; j++) {
            EXPECT_EQ(send_buffer[j], recv_buffer[j]);
          }
        }
        if (read_byte < 0)
          break;
      }

      loop++;
      if (loop_count != 0 && loop_count <= loop)
        break;
    }

    free(send_buffer);
    free(recv_buffer);

    EXPECT_EQ(expect_size, total_size);

    close(client_socket);
    return 0;
  }
};

/**
 * Direction: Client -> Server
 *
 * In this case, the client will invoke `write` system call exactly N times, and
 * the server will invoke `read` system call exactly N times.  Thus, this test
 * will gracefully accept cases when the `close` system call is not implemented
 * correctly.
 */
TEST_F(TestEnv_Any, TestTransfer_Connect_Send_Symmetric) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_ADDR"] = host2_ip;
  connect_env["BUFFER_SIZE"] = "1024";
  connect_env["LOOP_COUNT"] = "128";
  connect_env["SENDER"] = "1";
  connect_env["EXPECT_SIZE"] = "131072";
  int client_pid =
      host1->addApplication<TestTransfer_Connect>(*host1, connect_env);

  accept_env["SENDER"] = "0";
  accept_env["BUFFER_SIZE"] = "1024";
  accept_env["LOOP_COUNT"] = "128";
  accept_env["EXPECT_SIZE"] = "131072";
  int server_pid =
      host2->addApplication<TestTransfer_Accept>(*host2, accept_env);

  host2->launchApplication(server_pid);
  host1->launchApplication(client_pid);

  this->runTest();
}

/**
 * Direction: Client -> Server
 *
 * In this case, the server does not know how many times the `write` system call
 * will be invoked by the client.  The server will indefinitly wait for the
 * `read` system call unless the `EOF` is received.  You should implement proper
 * `close` semantics to pass this test.
 */
TEST_F(TestEnv_Any, TestTransfer_Connect_Send_EOF) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_ADDR"] = host2_ip;
  connect_env["BUFFER_SIZE"] = "1024";
  connect_env["LOOP_COUNT"] = "128";
  connect_env["SENDER"] = "1";
  connect_env["EXPECT_SIZE"] = "131072";
  int client_pid =
      host1->addApplication<TestTransfer_Connect>(*host1, connect_env);

  accept_env["SENDER"] = "0";
  accept_env["BUFFER_SIZE"] = "1024";
  accept_env["LOOP_COUNT"] = "0";
  accept_env["EXPECT_SIZE"] = "131072";
  int server_pid =
      host2->addApplication<TestTransfer_Accept>(*host2, accept_env);

  host2->launchApplication(server_pid);
  host1->launchApplication(client_pid);

  this->runTest();
}

/**
 * Data direction: Server -> Client.
 *
 * Same as `TestTransfer_Connect_Send_Symmetric` but the data direction is
 * changed.
 */
TEST_F(TestEnv_Any, TestTransfer_Connect_Recv_Symmetric) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_ADDR"] = host2_ip;
  connect_env["BUFFER_SIZE"] = "1024";
  connect_env["LOOP_COUNT"] = "128";
  connect_env["SENDER"] = "0";
  connect_env["EXPECT_SIZE"] = "131072";
  int client_pid =
      host1->addApplication<TestTransfer_Connect>(*host1, connect_env);

  accept_env["SENDER"] = "1";
  accept_env["BUFFER_SIZE"] = "1024";
  accept_env["LOOP_COUNT"] = "128";
  accept_env["EXPECT_SIZE"] = "131072";
  int server_pid =
      host2->addApplication<TestTransfer_Accept>(*host2, accept_env);

  host2->launchApplication(server_pid);
  host1->launchApplication(client_pid);

  this->runTest();
}

/**
 * Data direction: Server -> Client.
 *
 * Same as `TestTransfer_Connect_Send_EOF` but the data direction is changed.
 */
TEST_F(TestEnv_Any, TestTransfer_Connect_Recv_EOF) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_ADDR"] = host2_ip;
  connect_env["BUFFER_SIZE"] = "1024";
  connect_env["LOOP_COUNT"] = "0";
  connect_env["SENDER"] = "0";
  connect_env["EXPECT_SIZE"] = "131072";
  int client_pid =
      host1->addApplication<TestTransfer_Connect>(*host1, connect_env);

  accept_env["SENDER"] = "1";
  accept_env["BUFFER_SIZE"] = "1024";
  accept_env["LOOP_COUNT"] = "128";
  accept_env["EXPECT_SIZE"] = "131072";
  int server_pid =
      host2->addApplication<TestTransfer_Accept>(*host2, accept_env);

  host2->launchApplication(server_pid);
  host1->launchApplication(client_pid);

  this->runTest();
}

/**
 * Data direction: Server -> Client.
 *
 * In this case, the server uses a very small buffer (128B) while the client
 * sends large packets (>=512B).  Assuming that a packet has 512B data, it will
 * be used to fill the read buffer of 4 consequent `read` system calls.
 */
TEST_F(TestEnv_Any, TestTransfer_Connect_Recv_SmallBuffer1) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_ADDR"] = host2_ip;
  connect_env["BUFFER_SIZE"] = "128";
  connect_env["LOOP_COUNT"] = "0";
  connect_env["SENDER"] = "0";
  connect_env["EXPECT_SIZE"] = "131072";
  int client_pid =
      host1->addApplication<TestTransfer_Connect>(*host1, connect_env);

  accept_env["SENDER"] = "1";
  accept_env["BUFFER_SIZE"] = "1024";
  accept_env["LOOP_COUNT"] = "128";
  accept_env["EXPECT_SIZE"] = "131072";
  int server_pid =
      host2->addApplication<TestTransfer_Accept>(*host2, accept_env);

  host2->launchApplication(server_pid);
  host1->launchApplication(client_pid);

  this->runTest();
}

/**
 * Data direction: Server -> Client.
 *
 * In this case, the server uses an extreamly small buffer (67B). This is only 3
 * bytes larger than the minimum size of the ethernet frame.  Unlike the
 * previous example, the small buffer size no longer divides the size of the
 * large client buffer without remainders.
 */
TEST_F(TestEnv_Any, TestTransfer_Connect_Recv_SmallBuffer2) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_ADDR"] = host2_ip;
  connect_env["BUFFER_SIZE"] = "67";
  connect_env["LOOP_COUNT"] = "0";
  connect_env["SENDER"] = "0";
  connect_env["EXPECT_SIZE"] = "64819";
  int client_pid =
      host1->addApplication<TestTransfer_Connect>(*host1, connect_env);

  accept_env["SENDER"] = "1";
  accept_env["BUFFER_SIZE"] = "53";
  accept_env["LOOP_COUNT"] = "1223";
  accept_env["EXPECT_SIZE"] = "64819";
  int server_pid =
      host2->addApplication<TestTransfer_Accept>(*host2, accept_env);

  host2->launchApplication(server_pid);
  host1->launchApplication(client_pid);

  this->runTest();
}

/**
 * Exactly as same as the `TestTransfer_Connect_Recv_Symmetric`
 */
TEST_F(TestEnv_Any, TestTransfer_Accept_Send_Symmetric) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  accept_env["BUFFER_SIZE"] = "1024";
  accept_env["LOOP_COUNT"] = "128";
  accept_env["SENDER"] = "1";
  accept_env["EXPECT_SIZE"] = "131072";
  int server_pid =
      host1->addApplication<TestTransfer_Accept>(*host1, accept_env);

  connect_env["CONNECT_ADDR"] = host1_ip;
  connect_env["SENDER"] = "0";
  connect_env["BUFFER_SIZE"] = "1024";
  connect_env["LOOP_COUNT"] = "128";
  connect_env["EXPECT_SIZE"] = "131072";
  int client_pid =
      host2->addApplication<TestTransfer_Connect>(*host2, connect_env);

  host1->launchApplication(server_pid);
  host2->launchApplication(client_pid);

  this->runTest();
}

/**
 * In `TestTransfer_Connect_Recv_EOF` test, the client sends the EOF signal.  In
 * this test, the server sends the EOF signal.
 */
TEST_F(TestEnv_Any, TestTransfer_Accept_Send_EOF) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  accept_env["BUFFER_SIZE"] = "1024";
  accept_env["LOOP_COUNT"] = "128";
  accept_env["SENDER"] = "1";
  accept_env["EXPECT_SIZE"] = "131072";
  int server_pid =
      host1->addApplication<TestTransfer_Accept>(*host1, accept_env);

  connect_env["CONNECT_ADDR"] = host1_ip;
  connect_env["SENDER"] = "0";
  connect_env["BUFFER_SIZE"] = "1024";
  connect_env["LOOP_COUNT"] = "0";
  connect_env["EXPECT_SIZE"] = "131072";
  int client_pid =
      host2->addApplication<TestTransfer_Connect>(*host2, connect_env);

  host1->launchApplication(server_pid);
  host2->launchApplication(client_pid);

  this->runTest();
}

/**
 * Same as the `TestTransfer_Connect_Send_Symmetric` test.
 */
TEST_F(TestEnv_Any, TestTransfer_Accept_Recv_Symmetric) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  accept_env["BUFFER_SIZE"] = "1024";
  accept_env["LOOP_COUNT"] = "128";
  accept_env["SENDER"] = "0";
  accept_env["EXPECT_SIZE"] = "131072";
  int server_pid =
      host1->addApplication<TestTransfer_Accept>(*host1, accept_env);

  connect_env["CONNECT_ADDR"] = host1_ip;
  connect_env["SENDER"] = "1";
  connect_env["BUFFER_SIZE"] = "1024";
  connect_env["LOOP_COUNT"] = "128";
  connect_env["EXPECT_SIZE"] = "131072";
  int client_pid =
      host2->addApplication<TestTransfer_Connect>(*host2, connect_env);

  host1->launchApplication(server_pid);
  host2->launchApplication(client_pid);

  this->runTest();
}

/**
 * In `TestTransfer_Connect_Send_EOF` test, the client sends the EOF signal.  In
 * this test, the server sends the EOF signal.
 */
TEST_F(TestEnv_Any, TestTransfer_Accept_Recv_EOF) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  accept_env["BUFFER_SIZE"] = "1024";
  accept_env["LOOP_COUNT"] = "0";
  accept_env["SENDER"] = "0";
  accept_env["EXPECT_SIZE"] = "131072";
  int server_pid =
      host1->addApplication<TestTransfer_Accept>(*host1, accept_env);

  connect_env["CONNECT_ADDR"] = host1_ip;
  connect_env["SENDER"] = "1";
  connect_env["BUFFER_SIZE"] = "1024";
  connect_env["LOOP_COUNT"] = "128";
  connect_env["EXPECT_SIZE"] = "131072";
  int client_pid =
      host2->addApplication<TestTransfer_Connect>(*host2, connect_env);

  host1->launchApplication(server_pid);
  host2->launchApplication(client_pid);

  this->runTest();
}

/**
 * Same as the `TestTransfer_Connect_Recv_SmallBuffer1` test except for the data
 * transfer direction.
 */
TEST_F(TestEnv_Any, TestTransfer_Accept_Recv_SmallBuffer1) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  accept_env["BUFFER_SIZE"] = "128";
  accept_env["LOOP_COUNT"] = "0";
  accept_env["SENDER"] = "0";
  accept_env["EXPECT_SIZE"] = "131072";
  int server_pid =
      host1->addApplication<TestTransfer_Accept>(*host1, accept_env);

  connect_env["CONNECT_ADDR"] = host1_ip;
  connect_env["SENDER"] = "1";
  connect_env["BUFFER_SIZE"] = "1024";
  connect_env["LOOP_COUNT"] = "128";
  connect_env["EXPECT_SIZE"] = "131072";
  int client_pid =
      host2->addApplication<TestTransfer_Connect>(*host2, connect_env);

  host1->launchApplication(server_pid);
  host2->launchApplication(client_pid);

  this->runTest();
}

/**
 * Same as the `TestTransfer_Accept_Recv_SmallBuffer2` test except for the data
 * transfer direction.
 */
TEST_F(TestEnv_Any, TestTransfer_Accept_Recv_SmallBuffer2) {
  std::unordered_map<std::string, std::string> accept_env;
  std::unordered_map<std::string, std::string> connect_env;

  int seed = rand();
  accept_env["RANDOM_SEED"] = seed;
  connect_env["RANDOM_SEED"] = seed;

  ipv4_t ip1 = host1->getIPAddr(0).value();
  ipv4_t ip2 = host2->getIPAddr(0).value();

  char str_buffer[128];
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip1[0], ip1[1],
           ip1[2], ip1[3]);
  std::string host1_ip(str_buffer);
  snprintf(str_buffer, sizeof(str_buffer), "%u.%u.%u.%u", ip2[0], ip2[1],
           ip2[2], ip2[3]);
  std::string host2_ip(str_buffer);

  accept_env["LISTEN_ADDR"] = "0.0.0.0";
  accept_env["LISTEN_PORT"] = "9999";
  accept_env["BACKLOG"] = "1";
  accept_env["LISTEN_TIME"] = "0";
  accept_env["ACCEPT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(1000, TimeUtil::USEC), TimeUtil::USEC);
  accept_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  connect_env["CONNECT_PORT"] = "9999";
  connect_env["CONNECT_TIME"] = TimeUtil::printTime(
      TimeUtil::makeTime(2000, TimeUtil::USEC), TimeUtil::USEC);
  connect_env["START_TIME"] =
      TimeUtil::printTime(TimeUtil::makeTime(1, TimeUtil::SEC), TimeUtil::USEC);

  accept_env["BUFFER_SIZE"] = "67";
  accept_env["LOOP_COUNT"] = "0";
  accept_env["SENDER"] = "0";
  accept_env["EXPECT_SIZE"] = "64819";
  int server_pid =
      host1->addApplication<TestTransfer_Accept>(*host1, accept_env);

  connect_env["CONNECT_ADDR"] = host1_ip;
  connect_env["SENDER"] = "1";
  connect_env["BUFFER_SIZE"] = "53";
  connect_env["LOOP_COUNT"] = "1223";
  connect_env["EXPECT_SIZE"] = "64819";
  int client_pid =
      host2->addApplication<TestTransfer_Connect>(*host2, connect_env);

  host1->launchApplication(server_pid);
  host2->launchApplication(client_pid);

  this->runTest();
}
