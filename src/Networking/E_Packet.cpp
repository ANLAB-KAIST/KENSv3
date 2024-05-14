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

Packet::Packet(UUID uuid, size_t maxSize)
    : buffer(maxSize), bufferSize(maxSize), dataSize(maxSize), packetID(uuid) {

  std::fill(this->buffer.begin(), this->buffer.end(), 0);
}

Packet::Packet(const Packet &other)
    : buffer(other.buffer), bufferSize(other.bufferSize),
      dataSize(other.dataSize), packetID(other.packetID) {}

Packet::Packet(Packet &&other) noexcept
    : buffer(std::move(other.buffer)), bufferSize(other.bufferSize),
      dataSize(other.dataSize), packetID(other.packetID) {
  other.dataSize = 0;
}

Packet &Packet::operator=(const Packet &other) {
  buffer = other.buffer;
  bufferSize = other.bufferSize;
  dataSize = other.dataSize;
  packetID = other.packetID;
  return *this;
}

Packet &Packet::operator=(Packet &&other) noexcept {
  buffer = std::move(other.buffer);
  bufferSize = std::move(other.bufferSize);
  dataSize = std::move(other.dataSize);
  packetID = std::move(other.packetID);
  return *this;
}

Packet::Packet(size_t maxSize) : Packet(allocatePacketUUID(), maxSize) {}

Packet::~Packet() { freePacketUUID(this->packetID); }

Packet Packet::clone() const {

  Packet pkt(this->bufferSize);
  pkt.setSize(this->dataSize);
  pkt.buffer = this->buffer;
  return pkt;
}

size_t Packet::writeData(size_t offset, const void *data, size_t length) {
  size_t actual_offset = std::min(offset, dataSize);
  size_t actual_write = std::min(length, dataSize - actual_offset);

  if (actual_write == 0)
    return 0;

  assert(data);
  memcpy(this->buffer.data() + actual_offset, data, length);
  return actual_write;
}
size_t Packet::readData(size_t offset, void *data, size_t length) const {
  size_t actual_offset = std::min(offset, dataSize);
  size_t actual_read = std::min(length, dataSize - actual_offset);

  if (actual_read == 0)
    return 0;

  assert(data);
  memcpy(data, buffer.data() + actual_offset, length);
  return actual_read;
}
size_t Packet::setSize(size_t size) {
  this->dataSize = std::min(size, this->bufferSize);
  return this->dataSize;
}
size_t Packet::getSize() const { return this->dataSize; }

void Packet::clearContext() {}

} // namespace E
