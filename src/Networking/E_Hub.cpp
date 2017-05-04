/*
 * E_Hub.cpp
 *
 *  Created on: 2014. 11. 10.
 *      Author: Keunhong Lee
 */


#include <E/Networking/E_Hub.hpp>
#include <E/Networking/E_Port.hpp>
#include <E/Networking/E_Packet.hpp>

namespace E
{

Hub::Hub(std::string name, NetworkSystem* system) : Link(name, system)
{

}

void Hub::packetArrived(Port* inPort, Packet* packet)
{
	for(Port* port : this->connectedPorts)
	{
		if(inPort != port)
		{
			Packet* newPacket = this->clonePacket(packet);
			this->sendPacket(port, newPacket);
		}
	}
	this->freePacket(packet);
}


}
