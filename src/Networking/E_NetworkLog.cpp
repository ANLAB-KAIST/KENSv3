/*
 * E_NetworkLog.cpp
 *
 *  Created on: 2014. 11. 9.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_NetworkLog.hpp>
#include <E/Networking/E_Networking.hpp>

namespace E
{

uint64_t NetworkLog::defaultLevel = (
		//(1 << SYSCALL_RAISED) |
		//(1 << SYSCALL_FINISHED) |
		//(1 << PACKET_ALLOC) |
		//(1 << PACKET_CLONE) |
		//(1 << PACKET_FREE) |
		//(1 << PACKET_TO_MODULE) |
		//(1 << PACKET_FROM_MODULE) |
		//(1 << PACKET_TO_HOST) |
		//(1 << PACKET_FROM_HOST) |
		//(1 << PACKET_QUEUE) |
		//(1 << TCP_LOG) |
		0UL
		);

NetworkLog::NetworkLog(NetworkSystem* system)
{
	this->level = defaultLevel;
	this->system = system;
}

NetworkLog::NetworkLog(NetworkSystem* system, uint64_t level)
{
	this->level = level;
	this->system = system;
}
NetworkLog::~NetworkLog()
{

}

void NetworkLog::print_log(uint64_t level, const char* format, ...)
{
	if(!(((1UL << level) & this->level)))
		return;
	printf("Time[%lu]\t", system->getCurrentTime());
	va_list args;
	va_start (args, format);
	vprintf (format, args);
	va_end (args);
	printf("\n");
	fflush(stdout);
}

}
