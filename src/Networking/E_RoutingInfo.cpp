/*
 * E_RoutingInfo.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_RoutingInfo.hpp>

namespace E
{


RoutingInfo::RoutingInfo()
{

}
RoutingInfo::~RoutingInfo()
{

}

void RoutingInfo::setIPAddr(const uint8_t* ip, int port)
{
	struct ip_entry entry;
	memcpy(entry.ip, ip, sizeof(entry.ip));
	entry.port = port;
	this->ip_vector.push_back(entry);
}
void RoutingInfo::setMACAddr(const uint8_t* mac, int port)
{
	struct mac_entry entry;
	memcpy(entry.mac, mac, sizeof(entry.mac));
	entry.port = port;
	this->mac_vector.push_back(entry);
}
void RoutingInfo::setARPTable(const uint8_t* mac, const uint8_t* ip)
{
	struct arp_entry entry;
	memcpy(entry.ip, ip, sizeof(entry.ip));
	memcpy(entry.mac, mac, sizeof(entry.mac));
	this->arp_vector.push_back(entry);
}
void RoutingInfo::setRoutingTable(const uint8_t* mask, int prefix, int port)
{
	struct route_entry entry;
	memcpy(entry.ip_mask, mask, sizeof(entry.ip_mask));
	entry.port = port;
	entry.prefix = prefix;
	this->route_vector.push_back(entry);
}

bool RoutingInfo::getIPAddr(uint8_t* ip_buffer, int port)
{
	for(auto entry : ip_vector)
	{
		if(entry.port == port)
		{
			memcpy(ip_buffer, entry.ip, sizeof(entry.ip));
			return true;
		}
	}
	return false;
}
bool RoutingInfo::getMACAddr(uint8_t* mac_buffer, int port)
{
	for(auto entry : mac_vector)
	{
		if(entry.port == port)
		{
			memcpy(mac_buffer, entry.mac, sizeof(entry.mac));
			return true;
		}
	}
	return false;
}
bool RoutingInfo::getARPTable(uint8_t* mac_buffer, const uint8_t* ipv4)
{
	for(auto entry : arp_vector)
	{
		if(memcmp(ipv4, entry.ip, sizeof(entry.ip)) == 0)
		{
			memcpy(mac_buffer, entry.mac, sizeof(entry.mac));
			return true;
		}
	}
	return false;
}
int RoutingInfo::getRoutingTable(const uint8_t* ip_addr)
{
	int current_prefix = 0;
	int selected_port = 0;
	for(auto entry : route_vector)
	{
		bool matches = true;
		for(int k=0; k<(entry.prefix / 8); k++)
		{
			if(ip_addr[k] != entry.ip_mask[k])
				matches = false;
		}
		int remaining = entry.prefix % 8;
		if(remaining > 0)
		{
			int remove = 8 - remaining;
			uint8_t full = 0xFF;
			full = full >> remove;
			full = full << remove;
			if((ip_addr[(entry.prefix / 8)] & full) != (entry.ip_mask[(entry.prefix / 8)] & full))
				matches = false;
		}

		if(matches && entry.prefix > current_prefix)
		{
			current_prefix = entry.prefix;
			selected_port = entry.port;
		}
	}
	return selected_port;
}

}
