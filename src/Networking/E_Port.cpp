/*
 * E_Port.cpp
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
#include <E/Networking/E_Port.hpp>

namespace E {

Port::Port(std::string name, NetworkSystem *system, Time propagationDelay,
           Size bps, bool limit_speed)
    : Module(system), NetworkModule(name, system), NetworkLog(system) {
  this->connected[0] = nullptr;
  this->connected[1] = nullptr;
  this->nextAvailable[0] = system->getCurrentTime();
  this->nextAvailable[1] = system->getCurrentTime();
  this->propagationDelay = propagationDelay;
  this->bps = bps;
  this->limit_speed = limit_speed;
}

Port::~Port() {}

void Port::setSpeedLimit(bool do_limit) { this->limit_speed = do_limit; }

void Port::setPortSpeed(Size bps) { this->bps = bps; }
Size Port::getPortSpeed() { return this->bps; }

void Port::setPropagationDelay(Time delay) { propagationDelay = delay; }

Module::Message *Port::messageReceived(Module *from, Module::Message *message) {
  NetworkModule *netFrom = dynamic_cast<NetworkModule *>(from);
  assert(netFrom);
  Message *portMessage = dynamic_cast<Message *>(message);
  assert(portMessage != nullptr);
  assert(portMessage->type == Port::PACKET_TO_PORT);

  NetworkLog::print_log(
      PACKET_FROM_MODULE,
      "Port [%s] received a packet [size:%zu] from module [%s]",
      this->getModuleName().c_str(), portMessage->packet.getSize(),
      netFrom->getModuleName().c_str());

  int destination = -1;
  if (this->connected[0] == from)
    destination = 1;
  else if (this->connected[1] == from)
    destination = 0;
  if (destination == -1 || this->connected[destination] == nullptr) {
    return nullptr;
  }

  Time current_time = this->getSystem()->getCurrentTime();
  Time trans_delay = 0;
  if (this->bps != 0)
    trans_delay =
        (((Real)portMessage->packet.getSize() * 8 * (1000 * 1000 * 1000UL)) /
         (Real)this->bps);
  Time available_time = this->nextAvailable[destination];
  if (current_time > available_time) {
    available_time = current_time;
  }
  available_time = this->nextAvailable[destination] =
      available_time + trans_delay;

  NetworkModule *netTo =
      dynamic_cast<NetworkModule *>(this->connected[destination]);
  assert(netTo != nullptr);
  NetworkLog::print_log(
      PACKET_TO_MODULE,
      "Port [%s] send a packet [size:%zu] to module [%s] with transmission "
      "delay [%" PRIu64 "], propagation delay [%" PRIu64 "]",
      this->getModuleName().c_str(), portMessage->packet.getSize(),
      netTo->getModuleName().c_str(), trans_delay, propagationDelay);

  Message *fromPortMessage = new Message(MessageType::PACKET_FROM_PORT,
                                         std::move(portMessage->packet));

  if (this->limit_speed)
    sendMessage(this->connected[destination], fromPortMessage,
                available_time + propagationDelay - current_time);
  else
    sendMessage(this->connected[destination], fromPortMessage,
                propagationDelay);

  return nullptr;
}

void Port::connect(Module *module) {
  int selected = -1;
  for (int k = 0; k < 2; k++) {
    if (this->connected[k] == nullptr) {
      this->connected[k] = module;
      selected = k;
      break;
    }
  }
  assert(selected != -1);
}
void Port::disconnect(Module *module) {
  int selected = -1;
  for (int k = 0; k < 2; k++) {
    if (this->connected[k] == module) {
      selected = k;
      break;
    }
  }

  assert(selected != -1);
  this->connected[selected] = nullptr;
}
Time Port::nextSendAvailable(Module *me) {
  int destination = -1;
  if (this->connected[0] == me)
    destination = 1;
  else if (this->connected[1] == me)
    destination = 0;
  if (destination == -1 || this->connected[destination] == nullptr) {
    assert(0);
    return 0;
  }

  return this->nextAvailable[destination];
}

void Port::messageFinished(Module *to, Module::Message *message,
                           Module::Message *response) {
  (void)to;
  assert(response == nullptr);
  delete message;
}

void Port::messageCancelled(Module *to, Module::Message *message) {
  (void)to;
  Message *portMessage = dynamic_cast<Message *>(message);
  assert(portMessage != nullptr);
  delete portMessage;
}

} // namespace E
