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

namespace E {

class TCPApplication : public SystemCallApplication {

public:
  TCPApplication(Host &host);
  virtual ~TCPApplication();

protected:
  virtual int socket(int domain, int type__unused, int protocol) final;
  virtual int close(int fd) final;
  virtual int bind(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen) final;
  virtual int getsockname(int sockfd, struct sockaddr *addr,
                          socklen_t *addrlen) final;
  virtual int getpeername(int sockfd, struct sockaddr *addr,
                          socklen_t *addrlen) final;
  virtual int read(int fd, void *buf, size_t count) final;
  virtual int write(int fd, const void *buf, size_t count) final;
  virtual int connect(int sockfd, const struct sockaddr *addr,
                      socklen_t addrlen) final;
  virtual int listen(int sockfd, int backlog) final;
  virtual int accept(int sockfd, struct sockaddr *addr,
                     socklen_t *addrlen) final;
  virtual int nsleep(uint64_t nanoseconds) final;
  virtual int usleep(uint64_t microsleep) final;
  virtual int msleep(uint64_t millisleep) final;
  virtual int sleep(uint64_t sleep) final;
  virtual int gettimeofday(struct timeval *tv, struct timezone *tz) final;
};

} // namespace E

#endif /* INCLUDE_E_NETWORKING_TCP_E_TCPAPPLICATION_HPP_ */
