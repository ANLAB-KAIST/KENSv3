/*
 * E_Scheduler.hpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */

#ifndef E_SCHEDULER_HPP_
#define E_SCHEDULER_HPP_


#include <E/E_Common.hpp>
#include <E/E_Module.hpp>

namespace E
{
class Processor;
class Job;
class Computer;
class Scheduler
{

public:
	enum MessageType
	{
		JOB_RAISED,
		JOB_FINISHED,
		TIMER,
	};
	class Message : public Module::Message
	{
	public:
		enum MessageType type;
		Job* job;
		Processor* processor;
		void* arg;
	};

protected:
	Computer* computer;
	Scheduler();
	virtual ~Scheduler();

	virtual void jobRaised(Job* job) = 0;
	virtual void jobFinished(Job* job, Processor* processor) = 0;
	virtual void timerCallback(void* arg) = 0;
	virtual void setTimer(Time time, void* arg) final;
	virtual void cancelTimer() final;

	friend class Computer;
};


}


#endif /* E_SCHEDULER_HPP_ */
