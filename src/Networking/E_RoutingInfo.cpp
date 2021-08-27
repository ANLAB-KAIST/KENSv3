/*
 * E_RoutingInfo.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_RoutingInfo.hpp>

namespace E {

RoutingInfoInterface::RoutingInfoInterface(Host &host) : host(host) {}
void RoutingInfoInterface::setARPTable(const mac_t &mac, const ipv4_t &ipv4) {
  host.setARPTable(mac, ipv4);
}

void RoutingInfoInterface::setRoutingTable(const ipv4_t &mask, int prefix,
                                           int port) {
  host.setRoutingTable(mask, prefix, port);
}

std::optional<ipv4_t> RoutingInfoInterface::getIPAddr(int port) {
  return host.getIPAddr(port);
}
std::optional<mac_t> RoutingInfoInterface::getMACAddr(int port) {
  return host.getMACAddr(port);
}

std::optional<mac_t> RoutingInfoInterface::getARPTable(const ipv4_t &ipv4) {
  return host.getARPTable(ipv4);
}

int RoutingInfoInterface::getRoutingTable(const ipv4_t &ip_addr) {
  return host.getRoutingTable(ip_addr);
}

RoutingInfo::RoutingInfo() {}
RoutingInfo::~RoutingInfo() {}

void RoutingInfo::setIPAddr(const ipv4_t &ip, int port) {
  this->ip_vector.push_back({ip, port});
}
void RoutingInfo::setMACAddr(const mac_t &mac, int port) {
  this->mac_vector.push_back({mac, port});
}
void RoutingInfo::setARPTable(const mac_t &mac, const ipv4_t &ip) {
  this->arp_vector.push_back({ip, mac});
}
void RoutingInfo::setRoutingTable(const ipv4_t &mask, int prefix, int port) {
  this->route_vector.push_back({mask, prefix, port});
}

std::optional<ipv4_t> RoutingInfo::getIPAddr(int port) {
  for (auto entry : ip_vector) {
    if (entry.port == port) {
      return entry.ip;
    }
  }
  return {};
}
std::optional<mac_t> RoutingInfo::getMACAddr(int port) {
  for (auto entry : mac_vector) {
    if (entry.port == port) {
      return entry.mac;
    }
  }
  return {};
}
std::optional<mac_t> RoutingInfo::getARPTable(const ipv4_t &ipv4) {
  for (auto entry : arp_vector) {
    if (ipv4 == entry.ip) {
      return entry.mac;
    }
  }
  return {};
}
int RoutingInfo::getRoutingTable(const ipv4_t &ip_addr) {
  int current_prefix = 0;
  int selected_port = 0;
  for (auto entry : route_vector) {
    bool matches = true;
    for (int k = 0; k < (entry.prefix / 8); k++) {
      if (ip_addr[k] != entry.ip_mask[k])
        matches = false;
    }
    int remaining = entry.prefix % 8;
    if (remaining > 0) {
      int remove = 8 - remaining;
      uint8_t full = 0xFF;
      full = full >> remove;
      full = full << remove;
      if ((ip_addr[(entry.prefix / 8)] & full) !=
          (entry.ip_mask[(entry.prefix / 8)] & full))
        matches = false;
    }

    if (matches && entry.prefix > current_prefix) {
      current_prefix = entry.prefix;
      selected_port = entry.port;
    }
  }
  return selected_port;
}

} // namespace E
