/*
 * E_NetworkUtil.hpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#ifndef E_NETWORKUTIL_HPP_
#define E_NETWORKUTIL_HPP_

#include <E/E_Common.hpp>

namespace E
{

class NetworkUtil
{
private:
	NetworkUtil();
	virtual ~NetworkUtil();

public:
	static uint16_t one_sum(const uint8_t* buffer, size_t size);
	static uint16_t tcp_sum(uint32_t source, uint32_t dest, const uint8_t* tcp_seg, size_t length);
	static void UINT64ToArray(uint64_t val, uint8_t* array, int length);
	static uint64_t arrayToUINT64(const uint8_t* array, int length);
};

}

#endif /* E_NETWORKUTIL_HPP_ */
