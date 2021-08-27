/*
 * E_Hub.cpp
 *
 *  Created on: 2014. 11. 10.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_Hub.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_Wire.hpp>

namespace E {

Hub::Hub(std::string name, NetworkSystem &system) : Link(name, system) {}

void Hub::packetArrived(const ModuleID inWireID, Packet &&packet) {
  for (const ModuleID port : this->ports) {
    if (inWireID != port) {
      Packet newPacket = packet.clone();
      this->sendPacket(port, std::move(newPacket));
    }
  }
}

} // namespace E
