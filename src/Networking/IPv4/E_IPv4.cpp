/*
 * E_IPv4.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/IPv4/E_IPv4.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_NetworkUtil.hpp>
#include <arpa/inet.h>
#include <netinet/ip.h>

namespace E
{

IPv4::IPv4(Host* host) : HostModule("IPv4", host),
		NetworkModule(this->getHostModuleName(), host->getNetworkSystem()), NetworkLog(host->getNetworkSystem())
{
	this->identification = 0;
}
IPv4::~IPv4()
{
}

void IPv4::packetArrived(std::string fromModule, Packet* packet)
{
	if(fromModule.compare("Ethernet") == 0)
	{
		{
			char first_byte, second_byte;
			packet->readData(12, &first_byte, 1);
			packet->readData(13, &second_byte, 1);

			assert(first_byte == 0x08 && second_byte == 0x00);
		}

		size_t ip_start = 14;
		uint8_t ip_header_buffer[20];
		packet->readData(ip_start, ip_header_buffer, 20);
		uint16_t checksum = NetworkUtil::one_sum(ip_header_buffer,20);
		if(checksum != 0xFFFF)
		{
			if(checksum != 0)
			{
				print_log(PROTOCOL_ERROR, "Wrong checksum. Non-zero %u", checksum);
				this->freePacket(packet);
				return;
			}
			print_log(PROTOCOL_WARNING, "Checksum should be negative zero %u", checksum);
		}

		uint8_t protocol;
		packet->readData(ip_start+9, &protocol, 1);
		if(protocol == 0x06) //TCP
		{
			this->sendPacket("TCP", packet);
		}
		else if(protocol == 0x11) //UDP
		{
			this->sendPacket("UDP", packet);
		}
		else
		{
			this->freePacket(packet);
		}
	}
	else if(fromModule.compare("TCP") == 0 || fromModule.compare("UDP") == 0)
	{
		uint8_t proto = 0;
		size_t ip_start = 14;
		if(fromModule.compare("TCP") == 0)
		{
			proto = 0x06;
		}
		if(fromModule.compare("UDP") == 0)
		{
			proto = 0x11;
		}
		uint8_t buf;

		{
			buf = 0;
			buf += 4 << 4; //IPv4
			buf += 5; //hlen = 5
			packet->writeData(ip_start + 0, &buf, 1);
		}
		{
			buf = 0; //DSCP, ECN
			packet->writeData(ip_start + 1, &buf, 1);
		}
		{
			buf = 0;
			assert(packet->getSize() >= ip_start + 20);
			uint16_t size = packet->getSize() - ip_start;
			buf = size >> 8;
			packet->writeData(ip_start + 2, &buf, 1);

			buf = (uint8_t)(size & 0xFF);
			packet->writeData(ip_start + 3, &buf, 1);
		}
		{
			uint16_t cur_id = identification++;
			buf = 0;
			buf = cur_id >> 8;
			packet->writeData(ip_start + 4, &buf, 1);

			buf = (uint8_t)(cur_id & 0xFF);
			packet->writeData(ip_start + 5, &buf, 1);
		}
		{
			buf = 1 << 6; //Do not frag
			packet->writeData(ip_start + 6, &buf, 1);

			buf = 0; //offset = 0;
			packet->writeData(ip_start + 7, &buf, 1);
		}
		{
			buf = 64; //TTL
			packet->writeData(ip_start + 8, &buf, 1);

			buf = proto; //PROTOCOL
			packet->writeData(ip_start + 9, &buf, 1);
		}
		{
			buf = 0; //checksum
			packet->writeData(ip_start + 10, &buf, 1);
			packet->writeData(ip_start + 11, &buf, 1);
		}
		{
			//assume ip address is written
		}
		uint8_t ip_header_buffer[20];
		packet->readData(ip_start, ip_header_buffer, 20);
		uint16_t checksum = NetworkUtil::one_sum(ip_header_buffer,20);
		checksum = ~checksum;
		checksum = htons(checksum);
		packet->writeData(ip_start + 10, (uint8_t*)&checksum, 2);

		this->sendPacket("Ethernet", packet);
	}
	else
	{
		assert(0);
	}
}

}
