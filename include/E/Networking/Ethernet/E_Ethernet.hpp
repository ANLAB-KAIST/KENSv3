/*
 * E_Ethernet.hpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#ifndef E_ETHERNET_HPP_
#define E_ETHERNET_HPP_

#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Host.hpp>

namespace E
{

class Ethernet : public HostModule, public NetworkModule, private NetworkLog
{
public:
	Ethernet(Host* host);
	virtual ~Ethernet();
protected:
	virtual void packetArrived(std::string fromModule, Packet* packet) final;
};

}

#endif /* E_ETHERNET_HPP_ */
