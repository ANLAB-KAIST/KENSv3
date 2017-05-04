/*
 * E_Computer.cpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/E_System.hpp>
#include <E/Scheduling/E_Computer.hpp>
#include <E/Scheduling/E_Job.hpp>
#include <E/Scheduling/E_Processor.hpp>
#include <E/Scheduling/E_Scheduler.hpp>


namespace E
{


Module::Message* Computer::messageReceived(Module* from, Module::Message* message)
{
	Message* comMessage = dynamic_cast<Message*>(message);

	assert(comMessage != nullptr);

	switch(comMessage->type)
	{
	case JOB_RUN:
	{
		Job* runningJob = comMessage->runningJob;
		print_log(Log::DEBUG, "Job run[%lu] %lu %lu %lu", this->getSystem()->getCurrentTime(),
				runningJob->raisedAt, runningJob->executionTime, runningJob->deadLine);
		Processor* curProc = dynamic_cast<Processor*>(from);
		assert(curProc != nullptr);

		curProc->isRunning = false;
		curProc->deleteJob();
		this->scheduler->jobFinished(runningJob, curProc);
		this->cancelJob(runningJob);

		break;
	}
	case JOB_CHECK:
	{
		Job* checkingJob = comMessage->checkingJob;
		print_log(Log::DEBUG, "Job check[%lu] %lu %lu %lu", this->getSystem()->getCurrentTime(),
				checkingJob->raisedAt, checkingJob->executionTime, checkingJob->deadLine);
		assert(checkingJob != nullptr);

		checkingJob->isChecking = false;
		this->scheduler->jobFinished(checkingJob, checkingJob->processor);
		if(checkingJob->processor != nullptr)
			checkingJob->processor->deleteJob();
		this->cancelJob(checkingJob);
		break;
	}
	case TIMER:
	{
		this->scheduler->timerCallback(comMessage->arg);
		break;
	}
	default:
		assert(0);
	}

	return nullptr;
}
void Computer::messageFinished(Module* to, Module::Message* message, Module::Message* response)
{
	delete message;
}
void Computer::messageCancelled(Module* to, Module::Message* message)
{
	delete message;
}

Computer::Computer(System* system, CPUID numCPU, Scheduler* scheduler, Time overhead) : Module(system), Log()
{
	this->scheduler = scheduler;
	for(CPUID k=0; k<numCPU; k++)
	{
		Processor* proc = new Processor(this, k, overhead);
		this->cpuVector.push_back(proc);
	}
	this->done = 0;
	this->miss = 0;
	this->raised = 0;

	this->isTimerSet = false;
	this->timerID = 0;
	scheduler->computer = this;
}
Computer::~Computer()
{
	for(auto cpu : this->cpuVector)
	{
		delete cpu;
	}
}

void Computer::setTimer(Time time, void* arg)
{
	if(this->isTimerSet)
		this->cancelMessage(this->timerID);
	Message* selfMessage = new Message;
	selfMessage->type = TIMER;
	selfMessage->arg = arg;
	this->isTimerSet = true;
	this->timerID = this->sendMessage(this, selfMessage, time);
}
void Computer::cancelTimer()
{
	if(this->isTimerSet)
		this->cancelMessage(this->timerID);
	this->timerID = 0;
	this->isTimerSet = false;
}

void Computer::raiseJob(Task* task, Time executionTime, Time deadline)
{
	Time current = this->getSystem()->getCurrentTime();
	assert(current <= deadline);
	Job * job = new Job(this->getSystem()->getCurrentTime(), executionTime, deadline, task);

	Message* selfMessage = new Message;
	selfMessage->type = JOB_CHECK;
	selfMessage->checkingJob = job;
	job->checkMessage = this->sendMessage(this, selfMessage, deadline - current);
	job->isChecking = true;

	this->scheduler->jobRaised(job);
}

void Computer::cancelJob(Job* job)
{
	if(job->processor != nullptr)
		job->processor->deleteJob();
	if(job->isChecking)
		this->cancelMessage(job->checkMessage);
	if(!job->isDone())
		this->miss++;
	else
		this->done++;
	delete job;
}

CPUID Computer::getNumCPU()
{
	return this->cpuVector.size();
}
size_t Computer::getDone()
{
	return this->done;
}
size_t Computer::getMiss()
{
	return this->miss;
}
size_t Computer::getRaised()
{
	return this->raised;
}

Processor* Computer::getCPU(CPUID cpuID)
{
	return this->cpuVector[cpuID];
}


}
