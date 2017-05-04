/*
 * E_Ethernet.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */


#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/Ethernet/E_Ethernet.hpp>
#include <E/Networking/E_Packet.hpp>

namespace E
{

Ethernet::Ethernet(Host* host) : HostModule("Ethernet", host),
		NetworkModule(this->getHostModuleName(), host->getNetworkSystem()), NetworkLog(host->getNetworkSystem())
{
}
Ethernet::~Ethernet()
{
}
void Ethernet::packetArrived(std::string fromModule, Packet* packet)
{
	if(fromModule.compare("Host") == 0)
	{
		char first_byte, second_byte;
		packet->readData(12, &first_byte, 1);
		packet->readData(13, &second_byte, 1);

		if(first_byte == 0x08 && second_byte == 0x00)
		{
			this->sendPacket("IPv4", packet);
		}
		else if(first_byte == 0x86 && second_byte == 0xDD)
		{
			this->sendPacket("IPv6", packet);
		}
		else
		{
			this->print_log(MODULE_ERROR, "Unsupported ethertype.");
			assert(0);
		}
	}
	else if(fromModule.compare("IPv4") == 0)
	{
		char first_byte = 0x08;
		char second_byte = 0x00;
		packet->writeData(12, &first_byte, 1);
		packet->writeData(13, &second_byte, 1);

		uint8_t dst_ip[4];
		packet->readData(30, dst_ip, 4);

		int port = this->getHost()->getRoutingTable(dst_ip);
		uint8_t src_mac[6];
		uint8_t dst_mac[6];
		this->getHost()->getMACAddr(src_mac, port);
		this->getHost()->getARPTable(dst_mac, dst_ip);

		packet->writeData(0, dst_mac, 6);
		packet->writeData(6, src_mac, 6);

		this->sendPacket("Host", packet);
	}
	else if(fromModule.compare("IPv6") == 0)
	{
		char first_byte = 0x86;
		char second_byte = 0xDD;
		packet->writeData(12, &first_byte, 1);
		packet->writeData(13, &second_byte, 1);
		this->sendPacket("Host", packet);
	}
	else
	{
		this->freePacket(packet);
	}
}



}

