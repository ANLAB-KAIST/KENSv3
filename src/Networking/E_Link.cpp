/*
 * E_Link.cpp
 *
 *  Created on: 2014. 11. 9.
 *      Author: Keunhong Lee
 */

#include <E/E_Common.hpp>
#include <E/E_TimeUtil.hpp>
#include <E/Networking/E_Link.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_Wire.hpp>

namespace E {

struct pcap_packet_header {
  uint32_t ts_sec;   /* timestamp seconds */
  uint32_t ts_usec;  /* timestamp microseconds */
  uint32_t incl_len; /* number of octets of packet saved in file */
  uint32_t orig_len; /* actual length of packet */
};

Module::Message Link::messageReceived(const ModuleID from,
                                      Module::MessageBase &message) {
  if (typeid(message) == typeid(Wire::Message &)) {
    Wire::Message &portMessage = dynamic_cast<Wire::Message &>(message);

    this->packetArrived(from, std::move(portMessage.packet));
  }

  if (typeid(message) == typeid(Link::Message &)) {
    Link::Message &selfMessage = dynamic_cast<Link::Message &>(message);
    if (selfMessage.type == CHECK_QUEUE) {
      const ModuleID wireID = selfMessage.wireID;
      std::list<Packet> &current_queue = this->outputQueue[wireID];
      assert(current_queue.size() > 0);
      Time current_time = this->getCurrentTime();
      Time &avail_time = this->nextAvailable[wireID];

      if (current_time >= avail_time) {
        Packet packet = current_queue.front();
        current_queue.pop_front();

        print_log(NetworkLog::PACKET_QUEUE,
                  "Output queue length for port[%s] decreased to [%zu]",
                  this->getModuleName(wireID).c_str(), current_queue.size());

        Time trans_delay = 0;
        if (this->bps != 0)
          trans_delay = (((Real)packet.getSize() * 8 * (1000 * 1000 * 1000UL)) /
                         (Real)this->bps);

        auto portMessage2 = std::make_unique<Wire::Message>(
            Wire::PACKET_TO_PORT, Packet(packet)); // explicit copy for pcap

        avail_time = current_time + trans_delay;

        if (pcap_enabled) {
          struct pcap_packet_header pcap_header;
          memset(&pcap_header, 0, sizeof(pcap_header));
          pcap_header.ts_sec = TimeUtil::getTime(current_time, TimeUtil::SEC);
          pcap_header.ts_usec =
              (TimeUtil::getTime(current_time, TimeUtil::NSEC) % 1000000000);
          pcap_header.incl_len = std::min(snaplen, packet.getSize());
          pcap_header.orig_len = packet.getSize();
          // nanosecond precision
          pcap_file.write((char *)&pcap_header, sizeof(pcap_header));

          std::vector<char> temp_buffer(pcap_header.incl_len);
          packet.readData(0, temp_buffer.data(), pcap_header.incl_len);
          pcap_file.write(temp_buffer.data(), pcap_header.incl_len);
        }

        this->sendMessage(wireID, std::move(portMessage2), trans_delay);

        if (current_queue.size() > 0) {
          Time wait_time = 0;
          if (avail_time > current_time)
            wait_time += (avail_time - current_time);
          assert(wait_time == trans_delay);
          auto selfMessage =
              std::make_unique<Link::Message>(Link::CHECK_QUEUE, wireID);

          this->sendMessageSelf(std::move(selfMessage), wait_time);
        }
      }
    }
  }

  return nullptr;
}
void Link::messageFinished(const ModuleID to, Module::Message message,
                           Module::MessageBase &response) {
  (void)to;
  assert(dynamic_cast<Module::EmptyMessage &>(response) ==
         Module::EmptyMessage::shared());
}

void Link::messageCancelled(const ModuleID to, Module::Message message) {}

void Link::sendPacket(const ModuleID port, Packet &&packet) {
  std::list<Packet> &current_queue = this->outputQueue[port];
  Time current_time = this->getCurrentTime();
  Time &avail_time = this->nextAvailable[port];

  if ((this->max_queue_length != 0) &&
      (current_queue.size() >= this->max_queue_length)) {
    // evict one
    Size min_drop = this->max_queue_length / 2;
    Size max_drop = this->max_queue_length;
    Real rand = this->rand_dist.nextDistribution(min_drop, max_drop);
    Size index = floor(rand);
    if (index >= this->max_queue_length)
      index = this->max_queue_length - 1;

    if (index < current_queue.size()) {
      auto iter = current_queue.begin();
      for (Size k = 0; k < index; k++) {
        ++iter;
      }
      assert(iter != current_queue.end());
      Packet toBeRemoved = *iter;
      current_queue.erase(iter);

      print_log(NetworkLog::PACKET_QUEUE,
                "Output queue for port[%s] is full, remove at %zu, packet "
                "length: %zu",
                this->getModuleName(port).c_str(), index,
                toBeRemoved.getSize());
    }
  }
  assert(this->max_queue_length == 0 ||
         current_queue.size() < this->max_queue_length);
  current_queue.push_back(packet);
  print_log(NetworkLog::PACKET_QUEUE,
            "Output queue length for port[%s] increased to [%zu]",
            this->getModuleName(port).c_str(), current_queue.size());
  if (current_queue.size() == 1) {
    Time wait_time = 0;
    if (avail_time > current_time)
      wait_time += (avail_time - current_time);
    auto selfMessage = std::make_unique<Link::Message>(Link::CHECK_QUEUE, port);
    this->sendMessageSelf(std::move(selfMessage), wait_time);
  }
}

void Link::setLinkSpeed(Size bps) { this->bps = bps; }

void Link::setQueueSize(Size max_queue_length) {
  this->max_queue_length = max_queue_length;
}

Link::Link(std::string name, NetworkSystem &system)
    : NetworkModule(system), NetworkLog(static_cast<System &>(system)) {
  this->bps = 1000000000;
  this->max_queue_length = 0;
  this->pcap_enabled = false;
  this->snaplen = 65535;
}
Link::~Link() {

  if (pcap_enabled) {
    pcap_enabled = false;
    pcap_file.close();
  }
}

struct pcap_file_header {
  uint32_t magic;
  uint16_t version_major;
  uint16_t version_minor;
  uint32_t thiszone; /* gmt to local correction */
  uint32_t sigfigs;  /* accuracy of timestamps */
  uint32_t snaplen;  /* max length saved portion of each pkt */
  uint32_t linktype; /* data link type (LINKTYPE_*) */
};

void Link::enablePCAPLogging(const std::string &filename, Size snaplen) {
  if (!pcap_enabled) {
    pcap_file.open(filename, std::ofstream::binary);
    pcap_enabled = true;
    this->snaplen = snaplen;

    struct pcap_file_header pcap_header;
    memset(&pcap_header, 0, sizeof(pcap_header));
    pcap_header.magic = 0xa1b23c4d; // nanosecond resolution
    pcap_header.version_major = 2;
    pcap_header.version_minor = 4;
    pcap_header.snaplen = snaplen;
    pcap_header.linktype = 1; // LINKTYPE_ETHERNET
    pcap_file.write((char *)&pcap_header, sizeof(pcap_header));
  }
}

} // namespace E
