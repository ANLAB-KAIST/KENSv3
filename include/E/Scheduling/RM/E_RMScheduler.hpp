/*
 * E_RMScheduler.hpp
 *
 *  Created on: 2014. 11. 3.
 *      Author: Keunhong Lee
 */

#ifndef E_RMSCHEDULER_HPP_
#define E_RMSCHEDULER_HPP_

#include <E/Scheduling/E_Scheduler.hpp>
#include <E/Scheduling/E_Job.hpp>

namespace E
{

class RMJobCompare
{
public:
	bool operator()(const Job* a , const Job* b )
	{
		Time a_period = a->getDeadLine() - a->getRaisedTime();
		Time b_period = b->getDeadLine() - b->getRaisedTime();
		if(a_period == b_period)
			return (const void*)a < (const void*)b;
		return a_period < b_period;
	}
};

class RMScheduler : public Scheduler, private Log
{
private:
	std::set<Job*, RMJobCompare> jobQueue;
protected:
	void jobFinished(Computer* computer, Processor* processor, Job* job);
	void jobRaised(Computer* computer, Job* job);
	void timerEvent() {};
public:
	RMScheduler();
	virtual ~RMScheduler();
};

}

#endif /* E_RMSCHEDULER_HPP_ */
