/**
 * @file   E_Link.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::Link
 */

#ifndef E_LINK_HPP_
#define E_LINK_HPP_

#include <E/E_Common.hpp>
#include <E/Networking/E_NetworkLog.hpp>
#include <E/Networking/E_Networking.hpp>
#include <fstream>
#include <E/E_RandomDistribution.hpp>

namespace E
{
class Port;
class Packet;

/**
 * @brief Link makes connections among multiple Port.
 * It supports packet switching and output queuing.
 * Random drop occurs when the queue is full.
 */
class Link : public Module, public NetworkModule, private NetworkLog
{
private:
	virtual Module::Message* messageReceived(Module* from, Module::Message* message) final;
	virtual void messageFinished(Module* to, Module::Message* message, Module::Message* response) final;
	virtual void messageCancelled(Module* to, Module::Message* message) final;

	std::ofstream pcap_file;
	bool pcap_enabled;
	Size snaplen;
	LinearDistribution rand_dist;
protected:
	std::unordered_set<Port*> connectedPorts;
	std::unordered_map<Port*, Time> nextAvailable;
	std::unordered_map<Port*, std::list<Packet*>> outputQueue;
	Size bps;
	Size max_queue_length;
	virtual void packetArrived(Port* port, Packet* packet) = 0;
	virtual void packetSent(Port* port, Packet* packet) {};
	virtual void sendPacket(Port* port, Packet* packet) final;
public:
	/**
	 * @see NetworkModule
	 */
	Link(std::string name, NetworkSystem* system);
	virtual ~Link();

	/**
	 * @brief Make a PCAP formatted log file.
	 * @param filename Name of the log file.
	 * @param snaplen Length of packet data to be recorded.
	 * @note You cannot override this function.
	 */
	virtual void enablePCAPLogging(const std::string &filename, Size snaplen = 65535) final;

	enum MessageType
	{
		CHECK_QUEUE,
	};
	class Message : public Module::Message
	{
	public:
		enum MessageType type;
		Port* port;
	};

	/**
	 * @param port Link a port to this link.
	 */
	virtual void addPort(Port* port) final;

	/**
	 * @param bps Set link speed to bps.
	 */
	virtual void setLinkSpeed(Size bps) final;

	/**
	 * @param max_queue_length Set the maximum queue length.
	 * Zero indicates infinite queue.
	 */
	virtual void setQueueSize(Size max_queue_length) final;
};

}



#endif /* E_LINK_HPP_ */
