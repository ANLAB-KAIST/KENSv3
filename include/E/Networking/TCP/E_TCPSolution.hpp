/*
 * E_TCPSolution.hpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#ifndef E_TCPSOLUTION_HPP_
#define E_TCPSOLUTION_HPP_

#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_TimerModule.hpp>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

namespace E {

class TCPSolutionProvider {
private:
  TCPSolutionProvider() {}
  ~TCPSolutionProvider() {}

public:
  static void allocate(Host &host, bool retransmission = true,
                       bool boundary_test = true,
                       bool congestion_control = true);
};

} // namespace E

#endif /* E_TCPSOLUTION_HPP_ */
