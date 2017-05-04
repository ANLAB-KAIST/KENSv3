/*
 * E_Task.cpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */




#include <E/E_System.hpp>
#include <E/E_Module.hpp>
#include <E/Scheduling/E_Task.hpp>
#include <E/Scheduling/E_Computer.hpp>
#include <E/Scheduling/E_Job.hpp>
#include <E/Scheduling/E_Scheduler.hpp>

namespace E
{

PeriodicTask::PeriodicTask(Computer* computer, Time period, Time executionTime, Time startOffset) : Module(computer->getSystem()), Task()
{
	this->period = period;
	this->executionTime = executionTime;
	this->computer = computer;

	Message* selfMessage = new Message;
	selfMessage->type = TIMER;

	this->sendMessage(this, selfMessage, startOffset);
}

PeriodicTask::~PeriodicTask()
{

}

Module::Message* PeriodicTask::messageReceived(Module* from, Module::Message* message)
{
	Message* selfMessage = dynamic_cast<Message*>(message);

	switch(selfMessage->type)
	{
	case TIMER:
	{
		computer->raiseJob(this, this->executionTime, this->getSystem()->getCurrentTime() + this->period);

		Message* selfMessage = new Message;
		selfMessage->type = TIMER;

		this->sendMessage(this, selfMessage, this->period);
		break;
	}
	default:
		assert(0);
	}

	return nullptr;
}
void PeriodicTask::messageCancelled(Module* to, Module::Message* message)
{
	delete message;
}

void PeriodicTask::messageFinished(Module* to, Module::Message* message, Module::Message* response)
{
	delete message;
}


SporadicTask::SporadicTask(Computer* computer, Time period, Time executionTime, Time startOffset) : Module(computer->getSystem()), Task()
{
	this->minPeriod = period;
	this->worstExecution = executionTime;
	this->computer = computer;
	this->startOffset = startOffset;

	Message* selfMessage = new Message;
	selfMessage->type = TIMER;

	this->sendMessage(this, selfMessage, startOffset);
}

SporadicTask::~SporadicTask()
{

}


Time SporadicTask::getMinPeriod()
{
	return minPeriod;
}
Time SporadicTask::getWorstExecution()
{
	return worstExecution;
}

Module::Message* SporadicTask::messageReceived(Module* from, Module::Message* message)
{
	Message* selfMessage = dynamic_cast<Message*>(message);

	switch(selfMessage->type)
	{
	case TIMER:
	{
		computer->raiseJob(this, this->worstExecution, this->getSystem()->getCurrentTime() + this->minPeriod);

		Message* selfMessage = new Message;
		selfMessage->type = TIMER;

		this->sendMessage(this, selfMessage, this->minPeriod);
		break;
	}
	default:
		assert(0);
	}

	return nullptr;
}
void SporadicTask::messageCancelled(Module* to, Module::Message* message)
{
	delete message;
}

void SporadicTask::messageFinished(Module* to, Module::Message* message, Module::Message* response)
{
	delete message;
}

}
