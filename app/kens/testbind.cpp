/*
 * testbind.cpp
 *
 *  Created on: Mar 11, 2015
 *      Author: cjpark
 */

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
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

using namespace E;

class TestBind_Simple : public TCPApplication {
public:
  TestBind_Simple(Host &host) : TCPApplication(host) {}

protected:
  int E_Main() {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    int ret = bind(fd, (struct sockaddr *)&addr, len);

    EXPECT_EQ(ret, 0);

    close(fd);

    return 0;
  }
};

TEST_F(TestEnv_Reliable, TestBind_Simple) {

  int pid = host1->addApplication<TestBind_Simple>(*host1);
  host1->launchApplication(pid);

  this->runTest();
}

class TestBind_DoubleBind : public TCPApplication {
public:
  TestBind_DoubleBind(Host &host) : TCPApplication(host) {}

protected:
  int E_Main() {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    int ret = bind(fd, (struct sockaddr *)&addr, len);

    EXPECT_EQ(ret, 0);

    addr.sin_port = htons(10000);

    // bind to port 10000 with INADDR_ADY (should fail, fd is already bound to
    // 9999)
    ret = bind(fd, (struct sockaddr *)&addr, len);

    EXPECT_NE(ret, 0);

    close(fd);

    return 0;
  }
};

TEST_F(TestEnv_Reliable, TestBind_DoubleBind) {
  int pid = host1->addApplication<TestBind_DoubleBind>(*host1);
  host1->launchApplication(pid);

  this->runTest();
}

class TestBind_GetSockName : public TCPApplication {
public:
  TestBind_GetSockName(Host &host) : TCPApplication(host) {}

protected:
  int E_Main() {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    int ret = bind(fd, (struct sockaddr *)&addr, len);

    EXPECT_EQ(ret, 0);

    struct sockaddr *addr2 = (struct sockaddr *)malloc(len * 2);
    socklen_t len2 = len * 2;

    memset(addr2, 0, len2);

    ret = getsockname(fd, addr2, &len2);

    EXPECT_EQ(ret, 0);
    EXPECT_EQ(memcmp(&addr, addr2, len), 0);

    free(addr2);
    close(fd);

    return 0;
  }
};

TEST_F(TestEnv_Reliable, TestBind_GetSockName) {

  int pid = host1->addApplication<TestBind_GetSockName>(*host1);
  host1->launchApplication(pid);

  this->runTest();
}

class TestBind_OverlapPort : public TCPApplication {
public:
  TestBind_OverlapPort(Host &host) : TCPApplication(host) {}

protected:
  int E_Main() {
    int fd1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    int ret = bind(fd1, (struct sockaddr *)&addr, len);

    EXPECT_EQ(ret, 0);

    int fd2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_addr.s_addr = inet_addr("192.168.0.7");
    addr.sin_port = htons(9999);

    ret = bind(fd2, (struct sockaddr *)&addr, len);

    EXPECT_NE(ret, 0);

    close(fd1);
    close(fd2);

    return 0;
  }
};

TEST_F(TestEnv_Reliable, TestBind_OverlapPort) {

  int pid = host1->addApplication<TestBind_OverlapPort>(*host1);
  host1->launchApplication(pid);

  this->runTest();
}

class TestBind_OverlapClosed : public TCPApplication {
public:
  TestBind_OverlapClosed(Host &host) : TCPApplication(host) {}

protected:
  int E_Main() {
    int fd1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    int ret = bind(fd1, (struct sockaddr *)&addr, len);

    EXPECT_EQ(ret, 0);

    close(fd1);

    int fd2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_addr.s_addr = inet_addr("192.168.0.7");
    addr.sin_port = htons(9999);

    ret = bind(fd2, (struct sockaddr *)&addr, len);

    EXPECT_EQ(ret, 0);

    close(fd2);

    return 0;
  }
};

TEST_F(TestEnv_Reliable, TestBind_OverlapClosed) {
  int pid = host1->addApplication<TestBind_OverlapClosed>(*host1);
  host1->launchApplication(pid);

  this->runTest();
}

class TestBind_DifferentIP_SamePort : public TCPApplication {
public:
  TestBind_DifferentIP_SamePort(Host &host) : TCPApplication(host) {}

protected:
  int E_Main() {
    int fd1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.0.7");
    addr.sin_port = htons(9999);

    int ret = bind(fd1, (struct sockaddr *)&addr, len);

    EXPECT_EQ(ret, 0);

    int fd2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_addr.s_addr = inet_addr("192.168.0.8");
    addr.sin_port = htons(9999);

    ret = bind(fd2, (struct sockaddr *)&addr, len);

    EXPECT_EQ(ret, 0);

    close(fd1);
    close(fd2);

    return 0;
  }
};

TEST_F(TestEnv_Reliable, TestBind_DifferentIP_SamePort) {
  int pid = host1->addApplication<TestBind_DifferentIP_SamePort>(*host1);
  host1->launchApplication(pid);

  this->runTest();
}

class TestBind_SameIP_DifferentPort : public TCPApplication {
public:
  TestBind_SameIP_DifferentPort(Host &host) : TCPApplication(host) {}

protected:
  int E_Main() {
    int fd1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    memset(&addr, 0, len);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.0.7");
    addr.sin_port = htons(9999);

    int ret = bind(fd1, (struct sockaddr *)&addr, len);

    EXPECT_EQ(ret, 0);

    int fd2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_addr.s_addr = inet_addr("192.168.0.7");
    addr.sin_port = htons(10000);

    ret = bind(fd2, (struct sockaddr *)&addr, len);

    EXPECT_EQ(ret, 0);

    close(fd1);
    close(fd2);

    return 0;
  }
};

TEST_F(TestEnv_Reliable, TestBind_SameIP_DifferentPort) {
  int pid = host1->addApplication<TestBind_SameIP_DifferentPort>(*host1);
  host1->launchApplication(pid);

  this->runTest();
}
