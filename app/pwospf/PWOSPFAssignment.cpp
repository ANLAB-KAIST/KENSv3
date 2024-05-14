/*
 * E_PWOSPFAssignment.cpp
 *
 */

#include <E/E_Common.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_NetworkUtil.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Packet.hpp>
#include <cerrno>

#include "PWOSPFAssignment.hpp"

namespace E {

PWOSPFAssignment::PWOSPFAssignment(Host &host)
    : HostModule("OSPF", host), RoutingInfoInterface(host),
      TimerModule("OSPF", host) {}

PWOSPFAssignment::~PWOSPFAssignment() {}

void PWOSPFAssignment::initialize() {}

void PWOSPFAssignment::finalize() {}

/**
 * @brief Query cost for a host
 *
 * @param ipv4 querying host's IP address
 * @return cost or -1 for no found host
 */
Size PWOSPFAssignment::pwospfQuery(const ipv4_t &ipv4) {
  // Implement below

  return -1;
}

void PWOSPFAssignment::packetArrived(std::string fromModule, Packet &&packet) {
  // Remove below
  (void)fromModule;
  (void)packet;
}

void PWOSPFAssignment::timerCallback(std::any payload) {
  // Remove below
  (void)payload;
}

} // namespace E
