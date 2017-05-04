/*
 * E_RMScheduler.cpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */


#include <E/E_System.hpp>
#include <E/Scheduling/E_Scheduler.hpp>
#include <E/Scheduling/E_Processor.hpp>
#include <E/Scheduling/E_Computer.hpp>
#include <E/Scheduling/RM/E_RMScheduler.hpp>

namespace E
{

RMScheduler::RMScheduler() : Scheduler(), Log()
{

}

RMScheduler::~RMScheduler()
{
	jobQueue.clear();
}


void RMScheduler::jobFinished(Computer* computer, Processor* processor, Job* job)
{
	if(!jobQueue.empty())
	{
		auto iter = jobQueue.begin();
		Job* current = *iter;
		jobQueue.erase(*iter);

		processor->assignJob(current);
	}
}
void RMScheduler::jobRaised(Computer* computer, Job* job)
{
	assert(computer);
	assert(job);

	assert(jobQueue.find(job) == jobQueue.end());
	jobQueue.insert(job);
	auto iter = jobQueue.begin();
	Job* current = *iter;
	jobQueue.erase(*iter);
	Processor* curProc = computer->getCPU(0);
	Job* existing = curProc->getCurrentJob();

	RMJobCompare compare;
	if(existing && !compare(existing, current))
	{
		jobQueue.insert(current);
	}
	else
	{
		if(existing != nullptr)
		{
			curProc->deleteJob();
			jobQueue.insert(existing);
		}
		curProc->assignJob(current);
	}
}

}
