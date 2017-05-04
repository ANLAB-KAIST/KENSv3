/*
 * E_Packet.cpp
 *
 *  Created on: 2014. 11. 9.
 *      Author: Keunhong Lee
 */

#include <E/E_Common.hpp>
#include <E/Networking/E_Packet.hpp>

namespace E
{


Packet::Packet(UUID uuid, size_t maxSize)
{
	this->packetID = uuid;
	this->bufferSize = maxSize;
	this->dataSize = maxSize;

	this->buffer = malloc(maxSize);
	memset(this->buffer, 0, maxSize);
}
Packet::~Packet()
{
	assert(this->buffer);
	free(this->buffer);
	this->buffer = nullptr;
}
size_t Packet::writeData(size_t offset, const void* data, size_t length)
{
	size_t actual_offset = std::min(offset, dataSize);
	size_t actual_write = std::min(length, dataSize - actual_offset);

	if(actual_write == 0)
		return 0;

	assert(data);
	memcpy((char*)this->buffer + actual_offset, data, length);
	return actual_write;
}
size_t Packet::readData(size_t offset, void* data, size_t length)
{
	size_t actual_offset = std::min(offset, dataSize);
	size_t actual_read = std::min(length, dataSize - actual_offset);

	if(actual_read == 0)
		return 0;

	assert(data);
	memcpy(data, (char*)this->buffer + actual_offset, length);
	return actual_read;

}
size_t Packet::setSize(size_t size)
{
	this->dataSize = std::min(size, this->bufferSize);
	return this->dataSize;
}
size_t Packet::getSize()
{
	return this->dataSize;
}

void Packet::clearContext()
{

}

}
