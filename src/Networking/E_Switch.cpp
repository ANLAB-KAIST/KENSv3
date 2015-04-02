/*
 * E_Switch.cpp
 *
 *  Created on: Mar 14, 2015
 *      Author: leeopop
 */

#include <E/Networking/E_Switch.hpp>
#include <E/Networking/E_Port.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_NetworkUtil.hpp>

namespace E
{

Switch::Switch(std::string name, NetworkSystem* system) : Link(name, system)
{

}

void Switch::addMACEntry(Port* toPort, uint8_t* mac)
{
	uint64_t mac_int =  NetworkUtil::arrayToUINT64(mac, 6);
	if(this->mac_table.find(toPort) == this->mac_table.end())
		this->mac_table[toPort] = std::unordered_set<uint64_t>();
	this->mac_table[toPort].insert(mac_int);
}

void Switch::packetArrived(Port* inPort, Packet* packet)
{
	uint8_t mac[6];
	uint8_t broadcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	bool found = false;
	packet->readData(0,mac,6);
	uint64_t broad_int = NetworkUtil::arrayToUINT64(broadcast, 6);
	uint64_t mac_int = NetworkUtil::arrayToUINT64(mac, 6);
	for(Port* port : this->connectedPorts)
	{
		if(inPort != port)
		{
			if( (mac_int == broad_int) || ( (this->mac_table.find(port) != this->mac_table.end()) &&
					(this->mac_table[port].find(mac_int) != this->mac_table[port].end())) )
			{
				Packet* newPacket = this->clonePacket(packet);
				this->sendPacket(port, newPacket);
				found = true;
			}
		}
	}
	if(!found)
	{
		for(Port* port : this->connectedPorts)
		{
			if(inPort != port)
			{
				Packet* newPacket = this->clonePacket(packet);
				this->sendPacket(port, newPacket);
			}
		}
	}
	this->freePacket(packet);
}


}
