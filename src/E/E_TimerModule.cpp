/*
 * E_TimerModule.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */


#include <E/E_Module.hpp>
#include <E/E_TimerModule.hpp>

namespace E
{

TimerModule::TimerModule(System* system) : Module(system)
{

}
TimerModule::~TimerModule()
{

}

Module::Message* TimerModule::messageReceived(Module* from, Module::Message* message)
{
	Message* timerMessage = dynamic_cast<Message*>(message);
	assert(timerMessage != nullptr);

	this->timerCallback(timerMessage->payload);

	return nullptr;
}
void TimerModule::messageFinished(Module* to, Module::Message* message, Module::Message* response)
{
	delete message;
}
void TimerModule::messageCancelled(Module* to, Module::Message* message)
{
	delete message;
}


UUID TimerModule::addTimer(void* payload, Time timeAfter)
{
	Message* timerMessage = new Message;
	timerMessage->payload = payload;

	return this->sendMessage(this, timerMessage, timeAfter);
}

void TimerModule::cancelTimer(UUID key)
{
	this->cancelMessage(key);
}

}

