/*
 * E_Wire.cpp
 *
 *  Created on: 2014. 11. 9.
 *      Author: Keunhong Lee
 */

#include <E/E_Module.hpp>
#include <E/E_System.hpp>
#include <E/Networking/E_Host.hpp>
#include <E/Networking/E_Link.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_Wire.hpp>

namespace E {

Wire::Wire(std::string name, NetworkSystem &system, ModuleID left,
           ModuleID right, Time propagationDelay, Size bps, bool limit_speed)
    : Module(system), NetworkLog(static_cast<System &>(system)) {
  this->nextAvailable[0] = getCurrentTime();
  this->nextAvailable[1] = getCurrentTime();
  this->connected[0] = left;
  this->connected[1] = right;
  this->propagationDelay = propagationDelay;
  this->bps = bps;
  this->limit_speed = limit_speed;
}

Wire::~Wire() {}

void Wire::setSpeedLimit(bool do_limit) { this->limit_speed = do_limit; }

void Wire::setWireSpeed(Size bps) { this->bps = bps; }
Size Wire::getWireSpeed() { return this->bps; }

void Wire::setPropagationDelay(Time delay) { propagationDelay = delay; }

Module::Message Wire::messageReceived(const ModuleID from,
                                      Module::MessageBase &message) {

  Message &portMessage = dynamic_cast<Message &>(message);
  assert(portMessage.type == Wire::PACKET_TO_PORT);

  NetworkLog::print_log(
      NetworkLog::PACKET_FROM_MODULE,
      "Wire [%s] received a packet [size:%zu] from module [%s]",
      this->getModuleName().c_str(), portMessage.packet.getSize(),
      this->getModuleName(from).c_str());

  int destination = -1;
  if (this->connected[0] == from)
    destination = 1;
  else if (this->connected[1] == from)
    destination = 0;
  if (destination == -1 || this->connected[destination] == 0) {
    return nullptr;
  }

  Time current_time = this->getCurrentTime();
  Time trans_delay = 0;
  if (this->bps != 0)
    trans_delay =
        (((Real)portMessage.packet.getSize() * 8 * (1000 * 1000 * 1000UL)) /
         (Real)this->bps);
  Time available_time = this->nextAvailable[destination];
  if (current_time > available_time) {
    available_time = current_time;
  }
  available_time = this->nextAvailable[destination] =
      available_time + trans_delay;

  NetworkLog::print_log(
      NetworkLog::PACKET_TO_MODULE,
      "Wire [%s] send a packet [size:%zu] to module [%s] with transmission "
      "delay [%" PRIu64 "], propagation delay [%" PRIu64 "]",
      this->getModuleName().c_str(), portMessage.packet.getSize(),
      this->getModuleName(connected[destination]).c_str(), trans_delay,
      propagationDelay);

  auto fromWireMessage = std::make_unique<Message>(
      MessageType::PACKET_FROM_PORT, std::move(portMessage.packet));

  if (this->limit_speed)
    sendMessage(this->connected[destination], std::move(fromWireMessage),
                available_time + propagationDelay - current_time);
  else
    sendMessage(this->connected[destination], std::move(fromWireMessage),
                propagationDelay);

  return nullptr;
}

// void Wire::connect(const ModuleID module) {
//   int selected = -1;
//   for (int k = 0; k < 2; k++) {
//     if (this->connected[k] == nullptr) {
//       this->connected[k] = module;
//       selected = k;
//       break;
//     }
//   }
//   assert(selected != -1);
// }
// void Wire::disconnect(const ModuleID module) {
//   int selected = -1;
//   for (int k = 0; k < 2; k++) {
//     if (this->connected[k] == module) {
//       selected = k;
//       break;
//     }
//   }

//   assert(selected != -1);
//   this->connected[selected] = nullptr;
// }
Time Wire::nextSendAvailable(const ModuleID me) {
  int destination = -1;
  if (this->connected[0] == me)
    destination = 1;
  else if (this->connected[1] == me)
    destination = 0;
  if (destination == -1 || this->connected[destination] == 0) {
    assert(0);
    return 0;
  }

  return this->nextAvailable[destination];
}

void Wire::messageFinished(const ModuleID to, Module::Message message,
                           Module::MessageBase &response) {
  (void)to;
  assert(dynamic_cast<Module::EmptyMessage &>(response) ==
         Module::EmptyMessage::shared());
}

void Wire::messageCancelled(const ModuleID to, Module::Message message) {
  (void)to;
}

} // namespace E
