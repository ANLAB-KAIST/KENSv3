/*
 * E_Task.hpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */

#ifndef E_TASK_HPP_
#define E_TASK_HPP_


#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/Scheduling/E_Job.hpp>

namespace E
{
class Computer;

class Task
{
public:
	Task(){}
	virtual ~Task() {};
};

class PeriodicTask : public Module, public Task
{
private:
	Time period;
	Time executionTime;
	Computer* computer;

	virtual Module::Message* messageReceived(Module* from, Module::Message* message) final;
	virtual void messageFinished(Module* to, Module::Message* message, Module::Message* response) final;
	virtual void messageCancelled(Module* to, Module::Message* message) final;

public:
	PeriodicTask(Computer* computer, Time period, Time executionTime, Time startOffset);
	virtual ~PeriodicTask();

	enum MessageType
	{
		TIMER,
	};
	class Message : public Module::Message
	{
	public:
		enum MessageType type;
	};
};

class SporadicTask : public Module, public Task
{
protected:
	Time minPeriod;
	Time worstExecution;
	Time startOffset;

	Computer* computer;

private:
	virtual Module::Message* messageReceived(Module* from, Module::Message* message) final;
	virtual void messageFinished(Module* to, Module::Message* message, Module::Message* response) final;
	virtual void messageCancelled(Module* to, Module::Message* message) final;

public:
	SporadicTask(Computer* computer, Time period, Time executionTime, Time startOffset);
	virtual ~SporadicTask();

	enum MessageType
	{
		TIMER,
	};
	class Message : public Module::Message
	{
	public:
		enum MessageType type;
	};

	Time getMinPeriod();
	Time getWorstExecution();
};

}

#endif /* E_TASK_HPP_ */
