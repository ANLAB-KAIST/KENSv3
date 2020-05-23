/*
 * E_Link.cpp
 *
 *  Created on: 2014. 11. 9.
 *      Author: Keunhong Lee
 */

#include <E/E_Common.hpp>
#include <E/Networking/E_Link.hpp>
#include <E/Networking/E_Packet.hpp>
#include <E/Networking/E_Port.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/E_TimeUtil.hpp>

namespace E
{

struct pcap_packet_header {
	uint32_t ts_sec;         /* timestamp seconds */
	uint32_t ts_usec;        /* timestamp microseconds */
	uint32_t incl_len;       /* number of octets of packet saved in file */
	uint32_t orig_len;       /* actual length of packet */
};

Module::Message* Link::messageReceived(Module* from, Module::Message* message)
{
	Port::Message* portMessage = dynamic_cast<Port::Message*>(message);
	if(portMessage != nullptr)
	{
		assert(portMessage->packet != nullptr);
		Port* port = dynamic_cast<Port*>(from);
		assert(port != nullptr);

		this->packetArrived(port, portMessage->packet);
	}
	Link::Message* selfMessage = dynamic_cast<Link::Message*>(message);
	if(selfMessage != nullptr)
	{
		if(selfMessage->type == CHECK_QUEUE)
		{
			Port* port = selfMessage->port;
			std::list<Packet*> &current_queue = this->outputQueue[port];
			assert(current_queue.size() > 0);
			Time current_time = this->getSystem()->getCurrentTime();
			Time &avail_time = this->nextAvailable[port];

			if(current_time >= avail_time)
			{
				Packet* packet = current_queue.front();
				current_queue.pop_front();

				print_log(NetworkLog::PACKET_QUEUE, "Output queue length for port[%s] decreased to [%lu]",
						port->getModuleName().c_str(), current_queue.size());

				Time trans_delay = 0;
				if(this->bps != 0)
					trans_delay = (((Real)packet->getSize() * 8 * (1000*1000*1000UL)) / (Real)this->bps);

				Port::Message* portMessage = new Port::Message;
				portMessage->packet = packet;
				portMessage->type = Port::PACKET_TO_PORT;

				avail_time = current_time + trans_delay;

				if(pcap_enabled)
				{
					struct pcap_packet_header pcap_header;
					memset(&pcap_header, 0, sizeof(pcap_header));
					pcap_header.ts_sec = TimeUtil::getTime(current_time, TimeUtil::SEC);
					pcap_header.ts_usec = (TimeUtil::getTime(current_time, TimeUtil::NSEC) % 1000000000);
					pcap_header.incl_len = std::min(snaplen, packet->getSize());
					pcap_header.orig_len = packet->getSize();
					//nanosecond precision
					pcap_file.write((char*)&pcap_header, sizeof(pcap_header));

					char* temp_buffer = new char[pcap_header.incl_len];
					packet->readData(0, temp_buffer, pcap_header.incl_len);
					pcap_file.write(temp_buffer, pcap_header.incl_len);
					delete[] temp_buffer;
				}

				this->sendMessage(port, portMessage, trans_delay);

				if(current_queue.size() > 0)
				{
					Time wait_time = 0;
					if(avail_time > current_time)
						wait_time += (avail_time - current_time);
					assert(wait_time == trans_delay);
					Link::Message* selfMessage = new Link::Message;
					selfMessage->port = port;
					selfMessage->type = Link::CHECK_QUEUE;
					this->sendMessage(this, selfMessage, wait_time);
				}
			}
		}
	}

	return nullptr;
}
void Link::messageFinished(Module* to, Module::Message* message, Module::Message* response)
{
	assert(response == nullptr);

	delete message;
}

void Link::messageCancelled(Module* to, Module::Message* message)
{
	Port::Message* portMessage = dynamic_cast<Port::Message*>(message);
	Port* port = dynamic_cast<Port*>(to);
	if(portMessage != nullptr && port != nullptr)
	{
		if(portMessage->type == Port::PACKET_TO_PORT)
		{
			freePacket(portMessage->packet);
		}
		delete portMessage;
	}
	else
		delete message;
}

void Link::sendPacket(Port* port, Packet* packet)
{
	std::list<Packet*> &current_queue = this->outputQueue[port];
	Time current_time = this->getSystem()->getCurrentTime();
	Time &avail_time = this->nextAvailable[port];

	if((this->max_queue_length != 0) && (current_queue.size() >= this->max_queue_length))
	{
		//evict one
		Size min_drop = this->max_queue_length/2;
		Size max_drop = this->max_queue_length;
		Real rand = this->rand_dist.nextDistribution(min_drop, max_drop);
		Size index = floor(rand);
		if(index >= this->max_queue_length)
			index = this->max_queue_length - 1;

		if(index < current_queue.size())
		{
			auto iter = current_queue.begin();
			for(Size k=0; k<index; k++)
			{
				++iter;
			}
			assert(iter != current_queue.end());
			Packet* toBeRemoved = *iter;
			current_queue.erase(iter);

			print_log(NetworkLog::PACKET_QUEUE, "Output queue for port[%s] is full, remove at %lu, packet length: %lu",
					port->getModuleName().c_str(), index, toBeRemoved->getSize());
			this->freePacket(toBeRemoved);
		}
	}
	assert(this->max_queue_length == 0 || current_queue.size() < this->max_queue_length);
	current_queue.push_back(packet);
	print_log(NetworkLog::PACKET_QUEUE, "Output queue length for port[%s] increased to [%lu]",
			port->getModuleName().c_str(), current_queue.size());
	if(current_queue.size() == 1)
	{
		Time wait_time = 0;
		if(avail_time > current_time)
			wait_time += (avail_time - current_time);
		Link::Message* selfMessage = new Link::Message;
		selfMessage->port = port;
		selfMessage->type = Link::CHECK_QUEUE;
		this->sendMessage(this, selfMessage, wait_time);
	}
}

void Link::setLinkSpeed(Size bps)
{
	this->bps = bps;
}

void Link::setQueueSize(Size max_queue_length)
{
	this->max_queue_length = max_queue_length;
}

Link::Link(std::string name, NetworkSystem* system) : Module(system), NetworkModule(name, system), NetworkLog(system)
{
	this->bps = 1000000000;
	this->max_queue_length = 0;
	this->pcap_enabled = false;
	this->snaplen = 65535;
}
Link::~Link()
{
	for(auto port : connectedPorts)
		port->disconnect(this);

	if(pcap_enabled)
	{
		pcap_enabled = false;
		pcap_file.close();
	}
}

void Link::addPort(Port* port)
{
	this->connectedPorts.insert(port);
	this->nextAvailable[port] = this->getSystem()->getCurrentTime();
	this->outputQueue[port] = std::list<Packet*>();
	port->connect(this);
}

struct pcap_file_header {
	uint32_t magic;
	uint16_t version_major;
	uint16_t version_minor;
	uint32_t thiszone;     /* gmt to local correction */
	uint32_t sigfigs;    /* accuracy of timestamps */
	uint32_t snaplen;    /* max length saved portion of each pkt */
	uint32_t linktype;   /* data link type (LINKTYPE_*) */
};

void Link::enablePCAPLogging(const std::string &filename, Size snaplen)
{
	if(!pcap_enabled)
	{
		pcap_file.open(filename);
		pcap_enabled = true;
		this->snaplen = snaplen;

		struct pcap_file_header pcap_header;
		memset(&pcap_header, 0, sizeof(pcap_header));
		pcap_header.magic = 0xa1b23c4d; //nanosecond resolution
		pcap_header.version_major = 2;
		pcap_header.version_minor = 4;
		pcap_header.snaplen = snaplen;
		pcap_header.linktype = 1;//LINKTYPE_ETHERNET
		pcap_file.write((char*)&pcap_header, sizeof(pcap_header));
	}
}

}
