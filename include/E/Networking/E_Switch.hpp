/*
 * E_Switch.hpp
 *
 *  Created on: Mar 14, 2015
 *      Author: leeopop
 */

#ifndef E_SWITCH_HPP_
#define E_SWITCH_HPP_

#include <E/Networking/E_Link.hpp>

namespace E
{

class Switch : public Link
{
private:
	std::unordered_map<Port*, std::unordered_set<uint64_t>> mac_table;
protected:
	virtual void packetArrived(Port* inPort, Packet* packet);
public:
	Switch(std::string name, NetworkSystem* system);

	void addMACEntry(Port* toPort, uint8_t* mac);
};

}

#endif /* E_SWITCH_HPP_ */
