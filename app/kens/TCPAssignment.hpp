/*
 * E_TCPAssignment.hpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#ifndef E_TCPASSIGNMENT_HPP_
#define E_TCPASSIGNMENT_HPP_

#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Networking.hpp>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <E/E_TimerModule.hpp>

namespace E {

class TCPAssignment : public HostModule,
                      public NetworkModule,
                      public SystemCallInterface,
                      private NetworkLog,
                      private TimerModule {
private:
private:
  virtual void timerCallback(std::any payload) final;

public:
  TCPAssignment(Host *host);
  virtual void initialize();
  virtual void finalize();
  virtual ~TCPAssignment();

protected:
  virtual void systemCallback(UUID syscallUUID, int pid,
                              const SystemCallParameter &param) final;
  virtual void packetArrived(std::string fromModule, Packet &&packet) final;
};

class TCPAssignmentProvider {
private:
  TCPAssignmentProvider() {}
  ~TCPAssignmentProvider() {}

public:
  static HostModule *allocate(Host *host) { return new TCPAssignment(host); }
};

} // namespace E

#endif /* E_TCPASSIGNMENT_HPP_ */
