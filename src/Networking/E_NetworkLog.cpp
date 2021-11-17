/*
 * E_NetworkLog.cpp
 *
 *  Created on: 2014. 11. 9.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_NetworkLog.hpp>
#include <E/Networking/E_Networking.hpp>

namespace E {

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
    0UL);

NetworkLog::NetworkLog(System &system) : system(system) {
  this->level = defaultLevel;
}

NetworkLog::NetworkLog(System &system, uint64_t level) : system(system) {
  this->level = level;
}
NetworkLog::~NetworkLog() {}

void NetworkLog::print_log(uint64_t level, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprint_log(level, format, args);
  va_end(args);
}

void NetworkLog::vprint_log(uint64_t level, const char *format, va_list args) {
  if (!(((1UL << level) & this->level)))
    return;
  printf("Time[%" PRIu64 "]\t", system.getCurrentTime());
  vprintf(format, args);
  printf("\n");
  fflush(stdout);
}

} // namespace E
