/*
 * E_Networking.cpp
 *
 *  Created on: 2014. 11. 9.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Packet.hpp>

namespace E {

NetworkSystem::NetworkSystem() : System(), NetworkLog(this) {
  this->packetUUIDStart = 0;
}

NetworkSystem::~NetworkSystem() {}

NetworkModule::NetworkModule(std::string name, NetworkSystem *system) {
  this->system = system;
  this->name = name;
}

NetworkModule::~NetworkModule() {}

NetworkSystem *NetworkModule::getNetworkSystem() { return this->system; }

std::string NetworkModule::getModuleName() { return this->name; }

} // namespace E
