/*
 * E_TCPAssignment.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#include "TCPAssignment.hpp"
#include <E/E_Common.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_NetworkUtil.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Packet.hpp>
#include <cerrno>

namespace E {

TCPAssignment::TCPAssignment(Host &host)
    : HostModule("TCP", host), RoutingInfoInterface(host),
      SystemCallInterface(AF_INET, IPPROTO_TCP, host),
      TimerModule("TCP", host) {}

TCPAssignment::~TCPAssignment() {}

void TCPAssignment::initialize() {}

void TCPAssignment::finalize() {}

void TCPAssignment::systemCallback(UUID syscallUUID, int pid,
                                   const SystemCallParameter &param) {

  // Remove below
  (void)syscallUUID;
  (void)pid;

  switch (param.syscallNumber) {
  case SOCKET:
    // this->syscall_socket(syscallUUID, pid, param.param1_int,
    // param.param2_int, param.param3_int);
    break;
  case CLOSE:
    // this->syscall_close(syscallUUID, pid, param.param1_int);
    break;
  case READ:
    // this->syscall_read(syscallUUID, pid, param.param1_int, param.param2_ptr,
    // param.param3_int);
    break;
  case WRITE:
    // this->syscall_write(syscallUUID, pid, param.param1_int, param.param2_ptr,
    // param.param3_int);
    break;
  case CONNECT:
    // this->syscall_connect(syscallUUID, pid, param.param1_int,
    //		static_cast<struct sockaddr*>(param.param2_ptr),
    //(socklen_t)param.param3_int);
    break;
  case LISTEN:
    // this->syscall_listen(syscallUUID, pid, param.param1_int,
    // param.param2_int);
    break;
  case ACCEPT:
    // this->syscall_accept(syscallUUID, pid, param.param1_int,
    //		static_cast<struct sockaddr*>(param.param2_ptr),
    //		static_cast<socklen_t*>(param.param3_ptr));
    break;
  case BIND:
    // this->syscall_bind(syscallUUID, pid, param.param1_int,
    //		static_cast<struct sockaddr *>(param.param2_ptr),
    //		(socklen_t) param.param3_int);
    break;
  case GETSOCKNAME:
    // this->syscall_getsockname(syscallUUID, pid, param.param1_int,
    //		static_cast<struct sockaddr *>(param.param2_ptr),
    //		static_cast<socklen_t*>(param.param3_ptr));
    break;
  case GETPEERNAME:
    // this->syscall_getpeername(syscallUUID, pid, param.param1_int,
    //		static_cast<struct sockaddr *>(param.param2_ptr),
    //		static_cast<socklen_t*>(param.param3_ptr));
    break;
  default:
    assert(0);
  }
}

void TCPAssignment::packetArrived(std::string fromModule, Packet &&packet) {
  // Remove below
  (void)fromModule;
  (void)packet;
}

void TCPAssignment::timerCallback(std::any payload) {
  // Remove below
  (void)payload;
}

} // namespace E
