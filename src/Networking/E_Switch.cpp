/*
 * E_Switch.cpp
 *
 *  Created on: Mar 14, 2015
 *      Author: leeopop
 */

#include <E/Networking/E_NetworkUtil.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_Switch.hpp>
#include <E/Networking/E_Wire.hpp>

namespace E {

Switch::Switch(std::string name, NetworkSystem &system, bool unreliable)
    : Link(name, system) {
  this->unreliable = unreliable;
  this->drop_base = 1.0;
  this->drop_base_diff = 0.1;
  this->drop_base_limit = 0.15;
  this->drop_base_final = 0.01;
}

void Switch::addMACEntry(int port, const mac_t &mac) {
  uint64_t mac_int = NetworkUtil::arrayToUINT64(mac);
  auto wireID = ports[port];
  if (this->mac_table.find(wireID) == this->mac_table.end())
    this->mac_table[wireID] = std::unordered_set<uint64_t>();
  this->mac_table[wireID].insert(mac_int);
}

void Switch::packetArrived(const ModuleID inWireID, Packet &&packet) {
  mac_t mac;
  mac_t broadcast = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  bool found = false;
  packet.readData(0, mac.data(), 6);
  uint64_t broad_int = NetworkUtil::arrayToUINT64(broadcast);
  uint64_t mac_int = NetworkUtil::arrayToUINT64(mac);
  for (const ModuleID wireID : this->ports) {
    if (inWireID != wireID) {
      if ((mac_int == broad_int) ||
          ((this->mac_table.find(wireID) != this->mac_table.end()) &&
           (this->mac_table[wireID].find(mac_int) !=
            this->mac_table[wireID].end()))) {
        found = true;
        bool drop = false;
        if (this->unreliable) {
          Real val = this->dist.nextDistribution(0.0, 1.0);
          if (this->drop_base < this->drop_base_limit)
            this->drop_base = this->drop_base_final;
          else
            this->drop_base -= this->drop_base_diff;

          if (val < this->drop_base)
            drop = true;
        }
        Packet newPacket = packet.clone();
        if (drop) {
          if (newPacket.getSize() >= (14 + 20 + 20 + 4)) {
            uint32_t data;
            newPacket.readData(14 + 20 + 20, &data, sizeof(data));

            if (data != 0xEEEEEEEE)
              data = 0xEEEEEEEE;
            else
              data = 0xEEEEEEEF;

            newPacket.writeData(14 + 20 + 20, &data, sizeof(data));
          } else if (newPacket.getSize() >= (14 + 20 + 20)) {
            uint16_t checksum;
            newPacket.readData(14 + 20 + 16, &checksum, sizeof(checksum));

            if (checksum != 0xEEEE)
              checksum = 0xEEEE;
            else
              checksum = 0xEEEF;

            newPacket.writeData(14 + 20 + 16, &checksum, sizeof(checksum));
          }
        }
        this->sendPacket(wireID, std::move(newPacket));
      }
    }
  }
  if (!found) {
    for (const ModuleID wireID : this->ports) {
      if (inWireID != wireID) {
        Packet newPacket = packet.clone();
        this->sendPacket(wireID, std::move(newPacket));
      }
    }
  }
}

} // namespace E
