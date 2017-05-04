/*
 * E_APAScheduler.cpp
 *
 *  Created on: 2014. 11. 27.
 *      Author: Keunhong Lee
 */

#include <E/E_System.hpp>
#include <E/Scheduling/E_Computer.hpp>
#include <E/Scheduling/E_Task.hpp>
#include <E/Scheduling/E_Job.hpp>
#include <E/Scheduling/E_Processor.hpp>
#include <E/Scheduling/E_Scheduler.hpp>
#include <E/Scheduling/APA/E_Affinity.hpp>
#include <E/Scheduling/APA/E_APAScheduler.hpp>

namespace E
{

APAWeakScheduler::APAWeakScheduler(Size maxTask) : Scheduler(), Log()
{
	this->maxTask = maxTask;
}

APAWeakScheduler::~APAWeakScheduler()
{
	this->jobQueue.clear();
}

void APAWeakScheduler::jobFinished(Job* job, Processor* processor)
{
	this->jobQueue.erase(job);
	while(schedule(computer));
}
void APAWeakScheduler::jobRaised(Job* job)
{
	assert(computer);
	assert(job);

	assert(jobQueue.find(job) == jobQueue.end());
	jobQueue.insert(job);

	while(schedule(computer));
}

bool APAWeakScheduler::schedule(Computer* computer)
{
	bool ret = false;
	std::list<Job*> failedJob;
	failedJob.clear();
	while(!jobQueue.empty())
	{
		auto iter = jobQueue.begin();
		Job* current = *iter;
		jobQueue.erase(iter);

		AffinityTask::JobCompare jobCompare;

		AffinityTask* task = dynamic_cast<AffinityTask*>(current->getTask());
		assert(task != nullptr);
		bool found = false;
		Processor* selectedProc = nullptr;
		Job* selectedJob = nullptr;

		for(auto cpu : task->getAffinity())
		{
			assert(cpu < computer->getNumCPU());
			Processor* curProc = computer->getCPU(cpu);
			Job* existing = curProc->getCurrentJob();
			if(existing == nullptr)
			{
				selectedProc = curProc;
				selectedJob = existing;
				found = true;
				break;
			}
			else
			{
				if(jobCompare(current, existing))
				{
					if(selectedJob == nullptr)
					{
						selectedProc = curProc;
						selectedJob = existing;
						found = true;
					}
					else if(jobCompare(selectedJob, existing))
					{
						selectedProc = curProc;
						selectedJob = existing;
						found = true;
					}
				}
			}
		}
		if(found)
		{
			if(selectedJob != nullptr)
			{
				selectedProc->deleteJob();
				if(!selectedJob->isDone())
				{
					assert(jobQueue.find(selectedJob) == jobQueue.end());
					this->jobQueue.insert(selectedJob);
				}
			}

			selectedProc->assignJob(current);
			ret = true;
			break;
		}
		else
		{
			failedJob.push_back(current);
		}
	}
	if(failedJob.size() > 0)
		for(Job* item : failedJob)
		{
			assert(dynamic_cast<AffinityTask*>(item->getTask()) != nullptr);
			assert(jobQueue.find(item) == jobQueue.end());
			jobQueue.insert(item);
		}

	return ret;
}

APAStrongScheduler::APAStrongScheduler(Size maxTask) : Scheduler(), Log()
{
	this->maxTask = maxTask;
}

APAStrongScheduler::~APAStrongScheduler()
{
	jobQueue.clear();
}

void APAStrongScheduler::jobFinished(Job* job, Processor* processor)
{
	if(jobQueue.find(job) != jobQueue.end())
		jobQueue.erase(job);
	while(schedule(computer));
}
void APAStrongScheduler::jobRaised(Job* job)
{
	assert(computer);
	assert(job);

	assert(jobQueue.find(job) == jobQueue.end());
	jobQueue.insert(job);

	while(schedule(computer));
}

std::list<void*> APAStrongScheduler::BFS(Computer* computer, Job* job, CPUID targetCPU)
{
	//procedure BFS(G,v) is
	std::list<void*> returnList;
	std::unordered_map<void*, void*> prevMap;
	bool reachable = false;

	std::queue<std::pair<bool, void*>> queue;//create a queue Q, true is Job, false is processor
	std::unordered_set<void*> visited; //create a set V
	visited.insert(job); //add v to V
	queue.push(std::pair<bool, void*>(true, job)); //enqueue v onto Q

	while(!queue.empty())//while Q is not empty loop
	{
		auto currentPair = queue.front(); //t ← Q.dequeue()
		queue.pop();

		if(currentPair.first == false)
		{
			Processor* proc = static_cast<Processor*>(currentPair.second);
			if(proc->getID() == targetCPU) //if t is what we are looking for then
			{
				//return t
				reachable = true;
				break;
			}
		}

		//for all edges e in G.adjacentEdges(t) loop
		if(currentPair.first == true)
		{
			Job* curJob = static_cast<Job*>(currentPair.second);
			AffinityTask* curTask = dynamic_cast<AffinityTask*>(curJob->getTask());
			assert(curTask != nullptr);
			for(CPUID adjacentCPU : curTask->getAffinity()) //u ← G.adjacentVertex(t,e)
			{
				Processor* nextCPU = computer->getCPU(adjacentCPU);
				if(visited.find(nextCPU) == visited.end()) //if u is not in V then
				{
					visited.insert(nextCPU); //add u to V
					queue.push(std::pair<bool, void*>(false,nextCPU)); //enqueue u onto Q

					assert(prevMap.find(nextCPU) == prevMap.end());
					prevMap.insert(std::pair<void*,void*>(nextCPU, curJob));
				}
			}
		}
		else
		{
			Processor* curProc = static_cast<Processor*>(currentPair.second);
			Job* nextJob = curProc->getCurrentJob();
			if(nextJob != nullptr) //u ← G.adjacentVertex(t,e)
			{
				if(visited.find(nextJob) == visited.end())
				{
					visited.insert(nextJob);
					queue.push(std::pair<bool, void*>(true, nextJob));

					assert(prevMap.find(nextJob) == prevMap.end());
					prevMap.insert(std::pair<void*,void*>(nextJob, curProc));
				}
			}
		}
	}

	if(reachable)
	{
		Processor* lastProc = computer->getCPU(targetCPU);
		void* current = lastProc;
		while(current != nullptr)
		{
			returnList.push_front(current);
			auto iter = prevMap.find(current);
			if(iter == prevMap.end())
				current = nullptr;
			else
				current = iter->second;
		}
	}

	return returnList;
}

bool APAStrongScheduler::scheduleSingle(Computer* computer, Job* job)
{
	AffinityTask::JobCompare compare;
	Job* candidateJob = nullptr;
	Processor* candidateCPU = nullptr;
	std::list<void*> candidateChain;
	bool found = false;
	for(CPUID candidateProcessorID = 0;  candidateProcessorID < computer->getNumCPU(); candidateProcessorID++)
	{
		std::list<void*> currentChain = BFS(computer, job, candidateProcessorID);
		if(currentChain.size() == 0)
			continue;

		assert(currentChain.size() > 0);
		assert(currentChain.size() % 2 == 0);
		assert(currentChain.front() == (void*)job);

		Processor* lastProcessor = static_cast<Processor*>(currentChain.back());
		Job* currentJob = lastProcessor->getCurrentJob();
		if(currentJob == nullptr)
		{
			candidateCPU = lastProcessor;
			candidateJob = currentJob;
			candidateChain = currentChain;
			found = true;
			break;
		}
		else if(compare(job, currentJob)) //Job to be scheduled can evict the end of the chain
		{
			if(!found)
			{
				candidateCPU = lastProcessor;
				candidateJob = currentJob;
				candidateChain = currentChain;
				found = true;
			}
			else
			{
				assert(candidateJob != nullptr);
				if(compare(candidateJob, currentJob)) // currentJob is cheaper
				{
					candidateCPU = lastProcessor;
					candidateJob = currentJob;
					candidateChain = currentChain;
					found = true;
				}
			}
		}
	}
	if(found)
	{
		assert(candidateChain.size() > 0);
		assert(candidateChain.size() % 2 == 0);
		assert(candidateChain.front() == (void*)job);
		assert(candidateChain.back() == (void*)candidateCPU);

		if(candidateJob != nullptr)
		{
			assert(candidateJob == candidateCPU->getCurrentJob());
			candidateCPU->deleteJob();
			if(!candidateJob->isDone())
			{
				assert(jobQueue.find(candidateJob) == jobQueue.end());
				jobQueue.insert(candidateJob);
			}
		}

		while(candidateChain.size() > 0)
		{
			Job* movingJob = static_cast<Job*>(candidateChain.front());
			candidateChain.pop_front();
			Processor* movingProcessor = static_cast<Processor*>(candidateChain.front());
			candidateChain.pop_front();
			assert(movingJob != nullptr);
			assert(movingProcessor != nullptr);

			if(movingProcessor != candidateCPU)
				movingProcessor->deleteJob();
			if(!movingJob->isDone())
				movingProcessor->assignJob(movingJob);
		}
	}
	return found;
}

bool APAStrongScheduler::schedule(Computer* computer)
{
	bool ret = false;
	std::list<Job*> failedJob;
	failedJob.clear();
	while(!jobQueue.empty())
	{
		auto iter = jobQueue.begin();
		Job* current = *iter;
		jobQueue.erase(iter);

		bool found = scheduleSingle(computer, current);
		if(found)
		{
			ret = true;
			break;
		}
		else
		{
			failedJob.push_back(current);
		}
	}
	if(failedJob.size() > 0)
		for(Job* item : failedJob)
		{
			assert(dynamic_cast<AffinityTask*>(item->getTask()) != nullptr);
			assert(jobQueue.find(item) == jobQueue.end());
			jobQueue.insert(item);
		}
	return ret;
}

}
