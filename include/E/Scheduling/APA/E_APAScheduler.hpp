/*
 * E_APAScheduler.hpp
 *
 *  Created on: 2014. 11. 27.
 *      Author: Keunhong Lee
 */

#ifndef E_APASCHEDULER_HPP_
#define E_APASCHEDULER_HPP_


#include <E/Scheduling/E_Scheduler.hpp>
#include <E/Scheduling/RM/E_RMScheduler.hpp>
#include <E/Scheduling/APA/E_Affinity.hpp>

namespace E
{

class APAWeakScheduler : public Scheduler, public Log
{
private:
	std::set<Job*, RMJobCompare> jobQueue;
	bool schedule(Computer* computer);
protected:
	void jobFinished(Job* job, Processor* processor);
	void jobRaised(Job* job);
	void timerCallback(void* arg) {};
public:
	APAWeakScheduler(Size maxTask);
	virtual ~APAWeakScheduler();

private:
	Size maxTask;
};

class APAStrongScheduler : public Scheduler, public Log
{
private:
	std::set<Job*, RMJobCompare> jobQueue;
	bool scheduleSingle(Computer* computer, Job* job);
	bool schedule(Computer* computer);
	static std::list<void*> BFS(Computer* computer, Job* job, CPUID targetCPU);
	//std::set<std::list<void*>> allPath(Computer* computer, Job* job);
protected:
	void jobFinished(Job* job, Processor* processor);
	void jobRaised(Job* job);
	void timerCallback(void* arg) {};
public:
	APAStrongScheduler(Size maxTask);
	virtual ~APAStrongScheduler();

private:
	Size maxTask;
};

}

#endif /* E_APASCHEDULER_HPP_ */
