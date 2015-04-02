/*
 * E_TCPApplication.cpp
 *
 *  Created on: 2014. 11. 24.
 *      Author: KHL
 */

#include <E/Networking/TCP/E_TCPApplication.hpp>

namespace E
{
TCPApplication::TCPApplication(SystemCallApplication* syscall)
{
	this->syscall = syscall;
}
TCPApplication::~TCPApplication()
{

}

int TCPApplication::socket(int domain, int type__unused, int protocol)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_int = domain;
	param.param2_int = type__unused;
	param.param3_int = protocol;
	param.syscallNumber = SystemCallInterface::SystemCall::SOCKET;
	int ret = syscall->E_Syscall(param);
	return ret;
}
int TCPApplication::close(int fd)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_int = fd;
	param.syscallNumber = SystemCallInterface::SystemCall::CLOSE;
	int ret = syscall->E_Syscall(param);
	return ret;
}
int TCPApplication::bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_int = sockfd;
	param.param2_ptr = (void*)addr;
	param.param3_int = addrlen;
	param.syscallNumber = SystemCallInterface::SystemCall::BIND;
	int ret = syscall->E_Syscall(param);
	return ret;
}
int TCPApplication::getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_int = sockfd;
	param.param2_ptr = (void*)addr;
	param.param3_ptr = (void*)addrlen;
	param.syscallNumber = SystemCallInterface::SystemCall::GETSOCKNAME;
	int ret = syscall->E_Syscall(param);
	return ret;
}
int TCPApplication::getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_int = sockfd;
	param.param2_ptr = (void*)addr;
	param.param3_ptr = (void*)addrlen;
	param.syscallNumber = SystemCallInterface::SystemCall::GETPEERNAME;
	int ret = syscall->E_Syscall(param);
	return ret;
}
int TCPApplication::read(int fd, void *buf, size_t count)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_int = fd;
	param.param2_ptr = (void*)buf;
	param.param3_int = (int)count;
	param.syscallNumber = SystemCallInterface::SystemCall::READ;
	int ret = syscall->E_Syscall(param);
	return ret;
}
int TCPApplication::write(int fd, const void *buf, size_t count)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_int = fd;
	param.param2_ptr = (void*)buf;
	param.param3_int = (int)count;
	param.syscallNumber = SystemCallInterface::SystemCall::WRITE;
	int ret = syscall->E_Syscall(param);
	return ret;
}
int TCPApplication::connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_int = sockfd;
	param.param2_ptr = (void*)addr;
	param.param3_int = addrlen;
	param.syscallNumber = SystemCallInterface::SystemCall::CONNECT;
	int ret = syscall->E_Syscall(param);
	return ret;
}
int TCPApplication::listen(int sockfd, int backlog)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_int = sockfd;
	param.param2_int = backlog;
	param.syscallNumber = SystemCallInterface::SystemCall::LISTEN;
	int ret = syscall->E_Syscall(param);
	return ret;
}
int TCPApplication::accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_int = sockfd;
	param.param2_ptr = (void*)addr;
	param.param3_ptr = (void*)addrlen;
	param.syscallNumber = SystemCallInterface::SystemCall::ACCEPT;
	int ret = syscall->E_Syscall(param);
	return ret;
}
int TCPApplication::nsleep(long nanosleep)
{
	SystemCallInterface::SystemCallParameter param;
	param.param1_long = nanosleep;
	param.syscallNumber = SystemCallInterface::SystemCall::NSLEEP;
	int ret = syscall->E_Syscall(param);
	return ret;
}

int TCPApplication::usleep(long microsleep)
{
	return nsleep(1000L*microsleep);
}

int TCPApplication::msleep(long millisleep)
{
	return usleep(1000L*millisleep);
}

int TCPApplication::sleep(long sleep)
{
	return msleep(1000L*sleep);
}

int TCPApplication::gettimeofday(struct timeval *tv, struct timezone *tz)
{
	SystemCallInterface::SystemCallParameter param;
	param.syscallNumber = SystemCallInterface::SystemCall::GETTIMEOFDAY;
	param.param1_ptr = (void*)tv;
	param.param2_ptr = (void*)tz;
	int ret = syscall->E_Syscall(param);
	return ret;
}

}
