/*
 * E_Module.cpp
 *
 *  Created on: 2014. 11. 1.
 *      Author: Keunhong Lee
 */

#include <E/E_Module.hpp>
#include <E/E_System.hpp>

namespace E
{

System* Module::getSystem()
{
	return this->system;
}

Module::Module(System* system)
{
	this->system = system;
	system->registerModule(this);
}

Module::~Module()
{
	system->unregisterModule(this);
}

bool Module::cancelMessage(UUID timer)
{
	return this->system->cancelMessage(timer);
}

UUID Module::sendMessage(Module* to, Module::Message* message, Time timeAfter)
{
	return system->sendMessage(this, to, message, timeAfter);
}

}
