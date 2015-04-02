/*
 * E_TCPApplication.hpp
 *
 *  Created on: 2014. 11. 24.
 *      Author: KHL
 */

#ifndef E_TCPAPPLICATION_HPP_
#define E_TCPAPPLICATION_HPP_

#include <E/Networking/E_Host.hpp>
#include <arpa/inet.h>

namespace E
{

class TCPApplication
{
private:
	SystemCallApplication* syscall;
public:
	TCPApplication(SystemCallApplication* syscall);
	virtual ~TCPApplication();

protected:
	virtual int socket(int domain, int type__unused, int protocol) final;
	virtual int close(int fd) final;
	virtual int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) final;
	virtual int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen) final;
	virtual int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen) final;
	virtual int read(int fd, void *buf, size_t count) final;
	virtual int write(int fd, const void *buf, size_t count) final;
	virtual int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) final;
	virtual int listen(int sockfd, int backlog) final;
	virtual int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) final;
	virtual int nsleep(long nanoseconds) final;
	virtual int usleep(long microsleep) final;
	virtual int msleep(long millisleep) final;
	virtual int sleep(long sleep) final;
	virtual int gettimeofday(struct timeval *tv, struct timezone *tz) final;
};


}


#endif /* INCLUDE_E_NETWORKING_TCP_E_TCPAPPLICATION_HPP_ */
