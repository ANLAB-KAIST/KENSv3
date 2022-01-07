/*
 * E_TCPApplication.cpp
 *
 *  Created on: 2014. 11. 24.
 *      Author: KHL
 */

#include <E/Networking/TCP/E_TCPApplication.hpp>

namespace E {
TCPApplication::TCPApplication(Host &host) : SystemCallApplication(host) {}
TCPApplication::~TCPApplication() {}

int TCPApplication::socket(int domain, int type__unused, int protocol) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = domain;
  param.params[1] = type__unused;
  param.params[2] = protocol;
  param.syscallNumber = SystemCallInterface::SystemCall::SOCKET;
  int ret = E_Syscall(param);
  return ret;
}
int TCPApplication::close(int fd) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = fd;
  param.syscallNumber = SystemCallInterface::SystemCall::CLOSE;
  int ret = E_Syscall(param);
  return ret;
}
int TCPApplication::bind(int sockfd, const struct sockaddr *addr,
                         socklen_t addrlen) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = sockfd;
  param.params[1] = (void *)addr;
  param.params[2] = (int)addrlen;
  param.syscallNumber = SystemCallInterface::SystemCall::BIND;
  int ret = E_Syscall(param);
  return ret;
}
int TCPApplication::getsockname(int sockfd, struct sockaddr *addr,
                                socklen_t *addrlen) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = sockfd;
  param.params[1] = (void *)addr;
  param.params[2] = (void *)addrlen;
  param.syscallNumber = SystemCallInterface::SystemCall::GETSOCKNAME;
  int ret = E_Syscall(param);
  return ret;
}
int TCPApplication::getpeername(int sockfd, struct sockaddr *addr,
                                socklen_t *addrlen) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = sockfd;
  param.params[1] = (void *)addr;
  param.params[2] = (void *)addrlen;
  param.syscallNumber = SystemCallInterface::SystemCall::GETPEERNAME;
  int ret = E_Syscall(param);
  return ret;
}
int TCPApplication::read(int fd, void *buf, size_t count) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = fd;
  param.params[1] = (void *)buf;
  param.params[2] = (int)count;
  param.syscallNumber = SystemCallInterface::SystemCall::READ;
  int ret = E_Syscall(param);
  return ret;
}
int TCPApplication::write(int fd, const void *buf, size_t count) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = fd;
  param.params[1] = (void *)buf;
  param.params[2] = (int)count;
  param.syscallNumber = SystemCallInterface::SystemCall::WRITE;
  int ret = E_Syscall(param);
  return ret;
}
int TCPApplication::connect(int sockfd, const struct sockaddr *addr,
                            socklen_t addrlen) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = sockfd;
  param.params[1] = (void *)addr;
  param.params[2] = (int)addrlen;
  param.syscallNumber = SystemCallInterface::SystemCall::CONNECT;
  int ret = E_Syscall(param);
  return ret;
}
int TCPApplication::listen(int sockfd, int backlog) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = sockfd;
  param.params[1] = backlog;
  param.syscallNumber = SystemCallInterface::SystemCall::LISTEN;
  int ret = E_Syscall(param);
  return ret;
}
int TCPApplication::accept(int sockfd, struct sockaddr *addr,
                           socklen_t *addrlen) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = sockfd;
  param.params[1] = (void *)addr;
  param.params[2] = (void *)addrlen;
  param.syscallNumber = SystemCallInterface::SystemCall::ACCEPT;
  int ret = E_Syscall(param);
  return ret;
}
int TCPApplication::nsleep(uint64_t nanosleep) {
  SystemCallInterface::SystemCallParameter param;
  param.params[0] = nanosleep;
  param.syscallNumber = SystemCallInterface::SystemCall::NSLEEP;
  int ret = E_Syscall(param);
  return ret;
}

int TCPApplication::usleep(uint64_t microsleep) {
  return nsleep(1000L * microsleep);
}

int TCPApplication::msleep(uint64_t millisleep) {
  return usleep(1000L * millisleep);
}

int TCPApplication::sleep(uint64_t sleep) { return msleep(1000UL * sleep); }

int TCPApplication::gettimeofday(struct timeval *tv, struct timezone *tz) {
  SystemCallInterface::SystemCallParameter param;
  param.syscallNumber = SystemCallInterface::SystemCall::GETTIMEOFDAY;
  param.params[0] = (void *)tv;
  param.params[1] = (void *)tz;
  int ret = E_Syscall(param);
  return ret;
}

} // namespace E
