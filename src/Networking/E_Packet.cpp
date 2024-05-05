/*
 * E_Packet.cpp
 *
 *  Created on: 2014. 11. 9.
 *      Author: Keunhong Lee
 */

#include <E/E_Common.hpp>
#include <E/Networking/E_Packet.hpp>

namespace E {

std::unordered_set<UUID> Packet::packetUUIDSet;
UUID Packet::packetUUIDStart = 0;
UUID Packet::allocatePacketUUID() {
  UUID candidate = packetUUIDStart;
  do {
    if (packetUUIDSet.find(candidate) == packetUUIDSet.end()) {
      packetUUIDStart = candidate + 1;
      packetUUIDSet.insert(candidate);
      return candidate;
    }
    candidate++;
  } while (candidate != packetUUIDStart);
  assert(0);
  return 0;
}
void Packet::freePacketUUID(UUID uuid) { packetUUIDSet.erase(uuid); }

Packet::Packet(UUID uuid, size_t size) : buffer(size), packetID(uuid) {

  std::fill(this->buffer.begin(), this->buffer.end(), 0);
}

Packet::Packet(const Packet &other)
    : buffer(other.buffer), packetID(other.packetID) {}

Packet::Packet(Packet &&other) noexcept
    : buffer(std::move(other.buffer)), packetID(other.packetID) {
  other.buffer.clear();
}

Packet &Packet::operator=(const Packet &other) {
  buffer = other.buffer;
  packetID = other.packetID;
  return *this;
}

Packet &Packet::operator=(Packet &&other) noexcept {
  buffer = std::move(other.buffer);
  packetID = std::move(other.packetID);
  return *this;
}

Packet::Packet(size_t size) : Packet(allocatePacketUUID(), size) {}

Packet::~Packet() { freePacketUUID(this->packetID); }

Packet Packet::clone() const {

  Packet pkt(this->buffer.size());
  pkt.buffer = this->buffer;
  return pkt;
}

size_t Packet::writeData(size_t offset, const void *data, size_t length) {
  size_t actual_offset = std::min(offset, buffer.size());
  size_t actual_write = std::min(length, buffer.size() - actual_offset);

  if (actual_write == 0)
    return 0;

  assert(data);
  memcpy(this->buffer.data() + actual_offset, data, length);
  return actual_write;
}
size_t Packet::readData(size_t offset, void *data, size_t length) const {
  size_t actual_offset = std::min(offset, buffer.size());
  size_t actual_read = std::min(length, buffer.size() - actual_offset);

  if (actual_read == 0)
    return 0;

  assert(data);
  memcpy(data, buffer.data() + actual_offset, length);
  return actual_read;
}
size_t Packet::setSize(size_t size) {
  buffer.resize(size);
  return buffer.size();
}
size_t Packet::getSize() const { return buffer.size(); }

UUID Packet::getUUID() const { return this->packetID; }

void Packet::clearContext() {}

} // namespace E
