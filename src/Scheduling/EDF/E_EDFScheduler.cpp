/*
 * E_EDFScheduler.cpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */


#include <E/E_System.hpp>
#include <E/Scheduling/E_Scheduler.hpp>
#include <E/Scheduling/E_Processor.hpp>
#include <E/Scheduling/E_Computer.hpp>
#include <E/Scheduling/EDF/E_EDFScheduler.hpp>

namespace E
{

EDFScheduler::EDFScheduler() : Scheduler(), Log(), jobQueue()
{

}

EDFScheduler::~EDFScheduler()
{
	jobQueue.clear();
}

void EDFScheduler::jobFinished(Computer* computer, Processor* processor, Job* job)
{
	if(!jobQueue.empty())
	{
		auto iter = jobQueue.begin();
		Job* current = *iter;
		jobQueue.erase(*iter);

		processor->assignJob(current);
	}
}
void EDFScheduler::jobRaised(Computer* computer, Job* job)
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

	EDFJobCompare compare;
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

