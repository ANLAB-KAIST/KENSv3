/*
 * E_Networking.cpp
 *
 *  Created on: 2014. 11. 9.
 *      Author: Keunhong Lee
 */


#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Packet.hpp>

namespace E
{

NetworkSystem::NetworkSystem() : System(), NetworkLog(this)
{
	this->packetUUIDStart = 0;
}

void NetworkSystem::freePacketUUID(UUID uuid)
{
	this->packetUUIDSet.erase(uuid);
}
UUID NetworkSystem::allocatePacketUUID()
{
	UUID candidate = packetUUIDStart;
	do
	{
		if(this->packetUUIDSet.find(candidate) == this->packetUUIDSet.end())
		{
			packetUUIDStart = candidate+1;
			this->packetUUIDSet.insert(candidate);
			return candidate;
		}
		candidate++;
	}while(candidate != packetUUIDStart);
	assert(0);
	return 0;
}

NetworkSystem::~NetworkSystem()
{

}

Packet* NetworkSystem::allocatePacket(NetworkModule* module, size_t maxSize)
{
	std::string name = module->getModuleName();
	UUID packetID = this->allocatePacketUUID();
	NetworkLog::print_log(NetworkLog::PACKET_ALLOC, "Packet [%lu] allocated in module [%s] with size [%lu]", packetID, name.c_str(), maxSize);
	Packet* packet = new Packet(packetID, maxSize);

	return packet;
}

Packet* NetworkSystem::clonePacket(NetworkModule* module, Packet* packet)
{
	std::string name = module->getModuleName();
	UUID packetID = this->allocatePacketUUID();
	NetworkLog::print_log(NetworkLog::PACKET_CLONE, "Packet [%lu] cloned in module [%s] from original packet [%lu]", packetID, name.c_str(), packet->packetID);
	Packet* newPacket = new Packet(packetID, packet->bufferSize);
	memcpy(newPacket->buffer, packet->buffer, packet->bufferSize);
	return newPacket;
}

void NetworkSystem::freePacket(NetworkModule* module, Packet* packet)
{
	std::string name = module->getModuleName();
	this->freePacketUUID(packet->packetID);
	NetworkLog::print_log(NetworkLog::PACKET_FREE, "Packet [%lu] freed in module [%s] with size [%lu]/[%lu]",
			packet->packetID, name.c_str(), packet->dataSize, packet->bufferSize);
	delete packet;
}

NetworkModule::NetworkModule(std::string name, NetworkSystem* system)
{
	this->system = system;
	this->name = name;
}

NetworkModule::~NetworkModule()
{
}

NetworkSystem* NetworkModule::getNetworkSystem()
{
	return this->system;
}

std::string NetworkModule::getModuleName()
{
	return this->name;
}

Packet* NetworkModule::allocatePacket(size_t maxSize)
{
	return system->allocatePacket(this, maxSize);
}

void NetworkModule::freePacket(Packet* packet)
{
	this->system->freePacket(this, packet);
}

Packet* NetworkModule::clonePacket(Packet* packet)
{
	return this->system->clonePacket(this, packet);
}


}
