/*
 * E_TCPSolution.hpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#ifndef E_TCPSOLUTION_HPP_
#define E_TCPSOLUTION_HPP_


#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Host.hpp>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <E/E_TimerModule.hpp>

namespace E
{

class TCPSolutionProvider
{
private:
	TCPSolutionProvider() {}
	~TCPSolutionProvider() {}
public:
	static HostModule* allocate(Host* host, bool retransmission = true, bool boundary_test = true, bool congestion_control = true);
};


}


#endif /* E_TCPSOLUTION_HPP_ */
