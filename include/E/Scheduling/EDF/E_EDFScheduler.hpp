/*
 * E_EDFScheduler.hpp
 *
 *  Created on: 2014. 11. 3.
 *      Author: Keunhong Lee
 */

#ifndef E_EDFSCHEDULER_HPP_
#define E_EDFSCHEDULER_HPP_

#include <E/Scheduling/E_Scheduler.hpp>
#include <E/Scheduling/E_Job.hpp>

namespace E
{

class EDFJobCompare
{
public:
	bool operator()(const Job* a , const Job* b )
	{
		Time a_deadLine = a->getDeadLine();
		Time b_deadLine = b->getDeadLine();
		if(a_deadLine == b_deadLine)
			return (const void*)a < (const void*)b;
		return a_deadLine < b_deadLine;
	}
};

class EDFScheduler : public Scheduler, public Log
{
	std::set<Job*, EDFJobCompare> jobQueue;
protected:
	void jobFinished(Computer* computer, Processor* processor, Job* job);
	void jobRaised(Computer* computer, Job* job);
	void timerEvent() {};
public:
	EDFScheduler();
	virtual ~EDFScheduler();
};

}


#endif /* E_EDFSCHEDULER_HPP_ */
