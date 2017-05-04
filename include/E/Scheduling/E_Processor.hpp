/*
 * E_Processor.hpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */

#ifndef E_PROCESSOR_HPP_
#define E_PROCESSOR_HPP_

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>

namespace E
{

class Job;
class Computer;

class Processor : public Module, private Log
{
private:
	Time lastExecuted;
	Job* currentJob;
	CPUID id;
	Computer* computer;

	UUID currentRunID;
	bool isRunning;
	Time overhead;

	virtual Module::Message* messageReceived(Module* from, Module::Message* message) final;
	virtual void messageFinished(Module* to, Module::Message* message, Module::Message* response) final;
	virtual void messageCancelled(Module* to, Module::Message* message) final;
public:
	Processor(Computer* computer, CPUID id, Time overhead = 0);
	virtual ~Processor();
	Job* getCurrentJob();
	CPUID getID();
	void deleteJob();
	void assignJob(Job* job);
	Time getResource();

	friend class Job;
	friend class Computer;
};

}


#endif /* E_PROCESSOR_HPP_ */
