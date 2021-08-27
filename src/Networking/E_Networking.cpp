/*
 * E_Networking.cpp
 *
 *  Created on: 2014. 11. 9.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_Wire.hpp>
namespace E {

NetworkModule::NetworkModule(System &system) : Module(system) {}

int NetworkModule::connectWire(const ModuleID moduleID) {
  int portID = ports.size();
  ports.push_back(moduleID);
  return portID;
}

NetworkSystem::NetworkSystem()
    : System(), NetworkLog(static_cast<System &>(*this)) {
  this->packetUUIDStart = 0;
}

NetworkSystem::~NetworkSystem() {}

std::pair<std::shared_ptr<Wire>, std::pair<int, int>>
NetworkSystem::addWire(NetworkModule &left, NetworkModule &right,
                       Time propagationDelay, Size bps, bool limit_speed) {
  std::string wireName = "Wire [" + left.getModuleName() + " (" +
                         std::to_string(lookupModuleID(left)) + ") - " +
                         right.getModuleName() + " (" +
                         std::to_string(lookupModuleID(right)) + ")]";

  auto wire = addModule<Wire>(wireName, *this, lookupModuleID(left),
                              lookupModuleID(right), propagationDelay, bps,
                              limit_speed);
  int left_port_id = left.connectWire(lookupModuleID(*wire));
  int right_port_id = right.connectWire(lookupModuleID(*wire));
  return {wire, {left_port_id, right_port_id}};
}

Size NetworkSystem::getWireSpeed(const ModuleID moduleID) {
  auto &module = registeredModule[moduleID];
  auto &wire = dynamic_cast<Wire &>(*module);
  return wire.getWireSpeed();
}

} // namespace E
