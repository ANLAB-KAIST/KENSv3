/**
 * @file   E_Port.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::Port
 */

#ifndef E_PORT_HPP_
#define E_PORT_HPP_

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/Networking/E_NetworkLog.hpp>
#include <E/Networking/E_Networking.hpp>

#include <E/Networking/E_Link.hpp>
#include <E/Networking/E_Host.hpp>

namespace E
{
class Packet;
class Module;
class Host;

/**
 * @brief Port does a role of 2-ended wire.
 * However there is a speed limit and delayed packets are queued
 * (currently, no limitation to the queue length).
 */
class Port : public Module, public NetworkModule, private NetworkLog
{
private:
	Module* connected[2];
	Time nextAvailable[2];
	Time propagationDelay;
	Size bps;
	bool limit_speed;

public:
	/**
	 * @brief Create a Port.
	 * @param name See NetworkModule
	 * @param system See NetworkModule
	 * @param propagationDelay Propagation delay in nanoseconds.
	 * @param bps Link speed in bit per sec.
	 * @param limit_speed Control speed limiting functionality.
	 */
	Port(std::string name, NetworkSystem* system, Time propagationDelay = 1000000, Size bps = 1000000000UL, bool limit_speed = true);
	virtual ~Port();

	/**
	 * @param do_limit Control speed limiting functionality.
	 * @note You cannot override this function.
	 */
	virtual void setSpeedLimit(bool do_limit) final;

	/**
	 * @param bps Set data rate.
	 * @note You cannot override this function.
	 */
	virtual void setPortSpeed(Size bps) final;

	/**
	 * @param delay Set propagation delay.
	 * @note You cannot override this function.
	 */
	virtual void setPropagationDelay(Time delay) final;

	enum MessageType
	{
		PACKET_TO_PORT,
		PACKET_FROM_PORT,
	};

	class Message : public Module::Message
	{
	public:
		enum MessageType type;
		Packet* packet;
	};

	/**
	 * @brief Connect a module to one of this Port's port.
	 * Maximum 2 modules can be connected at once.
	 * @param module Module to be connected.
	 * @note You cannot override this function.
	 */
	virtual void connect(Module* module) final;

	/**
	 * @param module Module to be disconnected.
	 * @note You cannot override this function.
	 */
	virtual void disconnect(Module* module) final;
	virtual Time nextSendAvailable(Module* me) final;

private:
	virtual Module::Message* messageReceived(Module* from, Module::Message* message) final;
	virtual void messageFinished(Module* to, Module::Message* message, Module::Message* response) final;
	virtual void messageCancelled(Module* to, Module::Message* message) final;
};

}

#endif /* E_PORT_HPP_ */
