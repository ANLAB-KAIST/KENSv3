/*
 * E_TimerModule.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_TimerModule.hpp>

namespace E {

TimerModule::TimerModule(std::string name, Host &host)
    : host(host), name(name) {}
TimerModule::~TimerModule() {}

std::string TimerModule::getTimerModuleName() { return name; }

UUID TimerModule::addTimer(std::any payload, Time timeAfter) {
  return host.addTimer(name, payload, timeAfter);
}

void TimerModule::cancelTimer(UUID key) { host.cancelTimer(key); }

} // namespace E
