/*
 * E_Computer.hpp
 *
 *  Created on: 2014. 11. 1.
 *      Author: Keunhong Lee
 */

#ifndef E_COMPUTER_HPP_
#define E_COMPUTER_HPP_

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/E_Log.hpp>
#include <E/Scheduling/E_Scheduler.hpp>

namespace E
{
class Processor;
class Scheduler;
class Task;
class Job;
class Computer : public Module, private Log
{
private:

	UUID timerID;
	bool isTimerSet;

	std::vector<Processor*> cpuVector;
	Scheduler* scheduler;

	size_t done;
	size_t miss;
	size_t raised;

	virtual Module::Message* messageReceived(Module* from, Module::Message* message);
	virtual void messageFinished(Module* to, Module::Message* message, Module::Message* response);
	virtual void messageCancelled(Module* to, Module::Message* message);

	virtual void setTimer(Time time, void* arg) final;
	virtual void cancelTimer() final;

	friend void Scheduler::setTimer(Time time, void* arg);
	friend void Scheduler::cancelTimer();
	virtual void cancelJob(Job* job) final;
public:
	Computer(System* system, CPUID numCPU, Scheduler* scheduler, Time overhead = 0);
	virtual ~Computer();

	virtual void raiseJob(Task* task, Time executionTime, Time deadline) final;

	CPUID getNumCPU();
	size_t getDone();
	size_t getMiss();
	size_t getRaised();

	Processor* getCPU(CPUID cpuID);

	enum MessageType
	{
		JOB_CHECK,
		JOB_RUN,
		TIMER,
	};

	class Message : public Module::Message
	{
	public:
		enum MessageType type;
		union
		{
			Job* checkingJob;
			Job* runningJob;
			void* arg;
		};
	};
};

}

#endif /* E_COMPUTER_HPP_ */
