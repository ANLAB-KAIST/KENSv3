/*
 * E_NetworkUtil.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */


#include <E/Networking/E_NetworkUtil.hpp>
#include <arpa/inet.h>

namespace E
{

NetworkUtil::NetworkUtil()
{

}
NetworkUtil::~NetworkUtil()
{

}

uint16_t NetworkUtil::one_sum(const uint8_t* buffer, size_t size)
{
	bool upper = true;
	uint32_t sum = 0;
	for(size_t k=0; k<size; k++)
	{
		if(upper)
		{
			sum += buffer[k] << 8;
		}
		else
		{
			sum += buffer[k];
		}

		upper = !upper;

		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	sum = (sum & 0xFFFF) + (sum >> 16);
	return (uint16_t)sum;
}

struct pseudoheader
{
	uint32_t source;
	uint32_t destination;
	uint8_t zero;
	uint8_t protocol;
	uint16_t length;
}__attribute__((packed));

uint16_t NetworkUtil::tcp_sum(uint32_t source, uint32_t dest, const uint8_t* tcp_seg, size_t length)
{
	if(length < 20)
		return 0;
	struct pseudoheader pheader;
	pheader.source = source;
	pheader.destination = dest;
	pheader.zero = 0;
	pheader.protocol = IPPROTO_TCP;
	pheader.length = htons(length);

	uint32_t sum = one_sum((uint8_t*)&pheader, sizeof(pheader));
	sum += one_sum(tcp_seg, length);
	sum = (sum & 0xFFFF) + (sum >> 16);
	return (uint16_t)sum;
}

uint64_t NetworkUtil::arrayToUINT64(const uint8_t* array, int length)
{
	assert(length <= (int)sizeof(uint64_t));
	uint64_t sum = 0;
	for(int k=0; k<length; k++)
	{
		sum += (((uint64_t)array[k]) << (8 * k));
	}
	return sum;
}

void NetworkUtil::UINT64ToArray(uint64_t val, uint8_t* array, int length)
{
	assert(length <= (int)sizeof(uint64_t));
	for(int k=0; k<length; k++)
	{
		array[k] = (val >> (8*k)) & 0xFF;
	}
}

}
