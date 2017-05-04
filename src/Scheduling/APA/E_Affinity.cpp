/*
 * E_Affinity.cpp
 *
 *  Created on: 2014. 11. 26.
 *      Author: Keunhong Lee
 */

#include <E/Scheduling/E_Computer.hpp>
#include <E/Scheduling/APA/E_Affinity.hpp>
#include <E/Scheduling/E_Task.hpp>

namespace E
{


AffinityTask::AffinityTask(Affinity affinity, Computer* computer, Time period, Time executionTime, Time startOffset) :
				SporadicTask(computer, period, executionTime, startOffset)
{
	this->affinity = affinity;
}

AffinityTask::AffinityTask(const AffinityTask& orig, Computer* computer) :
				SporadicTask(computer, orig.minPeriod, orig.worstExecution, orig.startOffset)
{
	this->affinity = orig.affinity;
}

AffinityTask::~AffinityTask()
{

}

const Affinity& AffinityTask::AffinityTask::getAffinity()
{
	return this->affinity;
}

std::set<Affinity> AffinityTask::powerSet(const Affinity& affinity)
{
	assert(affinity.size() < 64);
	assert(affinity.size() > 0);
	std::set<Affinity> ret;
	std::vector<CPUID> listedAffinity;
	for(CPUID cpu : affinity)
		listedAffinity.push_back(cpu);

	uint64_t powerCount = 1 << (affinity.size());
	for(uint64_t k=1; k<powerCount; k++)
	{
		Affinity subset;
		for(uint64_t index=0; index<affinity.size(); index++)
		{
			if( (1UL << index) & k )
			{
				subset.insert(listedAffinity[index]);
			}
		}
		ret.insert(subset);
	}
	return ret;
}


GraphNode::GraphNode(AffinityTask* task)
{
	this->task = task;
	this->type = true;
}

GraphNode::GraphNode(CPUID cpu)
{
	this->cpu = cpu;
	this->type = false;
}

CPUID GraphNode::getCPUID() const
{
	assert(this->type == false);
	return this->cpu;
}
AffinityTask* GraphNode::getTask() const
{
	assert(this->type == true);
	return this->task;
}

bool GraphNode::isTask() const
{
	return this->type;
}

GraphNode::~GraphNode()
{

}

void AffinityTask::DFS(std::list<std::list<GraphNode>>& saveAt, const std::unordered_map<CPUID, TaskSet>& cpuToTaskList,
			const std::unordered_map<AffinityTask*, Affinity>& taskToCPUList,
			const GraphNode& start, const GraphNode& target, const std::list<GraphNode>& visited)
{
	std::list<GraphNode> currentVisited(visited);
	currentVisited.push_back(start);
	if(start.isTask() == target.isTask())
	{
		if(start.isTask())
			if(target.getTask() == start.getTask())
			{
				saveAt.push_back(currentVisited);
				return;
			}
		if(!start.isTask())
			if(target.getCPUID() == start.getCPUID())
			{
				saveAt.push_back(currentVisited);
				return;
			}
	}

	if(start.isTask())
	{
		auto iter = taskToCPUList.find(start.getTask());
		if(iter == taskToCPUList.end())
			return;
		for(CPUID adjacentCPU : iter->second) //u ← G.adjacentVertex(t,e)
		{
			bool ifVisited = false;
			for(auto item : visited)
			{
				if(!item.isTask())
				{
					if(item.getCPUID() == adjacentCPU)
					{
						ifVisited = true;
						break;
					}
				}
			}
			if(ifVisited)
				continue;
			DFS(saveAt, cpuToTaskList, taskToCPUList,
					adjacentCPU, target, currentVisited);
		}
	}
	else
	{
		auto iter = cpuToTaskList.find(start.getCPUID());
		if(iter == cpuToTaskList.end())
			return;
		for(AffinityTask* adjacentTask : iter->second) //u ← G.adjacentVertex(t,e)
		{
			bool ifVisited = false;
			for(auto item : visited)
			{
				if(item.isTask())
				{
					if(item.getTask() == adjacentTask)
					{
						ifVisited = true;
						break;
					}
				}
			}
			if(ifVisited)
				continue;
			DFS(saveAt, cpuToTaskList, taskToCPUList,
					adjacentTask, target, currentVisited);
		}
	}
}

std::list<std::list<GraphNode>> AffinityTask::allPath(const TaskSet& taskSet, const GraphNode& start, const GraphNode& target, const Affinity& excludeID, const TaskSet& excludeTask)
{
	std::list<std::list<GraphNode>> returnValue;
	//prepare link map
	std::unordered_map<CPUID, TaskSet> cpuToTaskList;
	std::unordered_map<AffinityTask*, Affinity> taskToCPUList;
	for(auto task : taskSet)
	{
		if(excludeTask.find(task) != excludeTask.end())
			continue;
		Affinity affinityCopy(task->affinity);
		for(auto cpu : excludeID)
			affinityCopy.erase(cpu);
		taskToCPUList.insert(std::pair<AffinityTask*, Affinity>(task, affinityCopy));
		for(CPUID cpu : affinityCopy)
		{
			if(cpuToTaskList.find(cpu) == cpuToTaskList.end())
				cpuToTaskList.insert(std::pair<CPUID, TaskSet>(cpu, TaskSet()));

			cpuToTaskList.find(cpu)->second.insert(task);
		}
	}

	DFS(returnValue, cpuToTaskList, taskToCPUList, start, target, std::list<GraphNode>());

	return returnValue;
}

std::list<GraphNode> AffinityTask::BFS(const TaskSet& taskSet, const GraphNode& start, const GraphNode& target, const Affinity& excludeID, const TaskSet& excludeTask)
{
	std::list<GraphNode> returnList;

	//prepare link map
	std::unordered_map<CPUID, TaskSet> cpuToTaskList;
	std::unordered_map<AffinityTask*, Affinity> taskToCPUList;
	for(auto task : taskSet)
	{
		if(excludeTask.find(task) != excludeTask.end())
			continue;
		Affinity affinityCopy(task->affinity);
		for(auto cpu : excludeID)
			affinityCopy.erase(cpu);
		taskToCPUList.insert(std::pair<AffinityTask*, Affinity>(task, affinityCopy));
		for(CPUID cpu : affinityCopy)
		{
			if(cpuToTaskList.find(cpu) == cpuToTaskList.end())
				cpuToTaskList.insert(std::pair<CPUID, TaskSet>(cpu, TaskSet()));

			cpuToTaskList.find(cpu)->second.insert(task);
		}
	}

	//procedure BFS(G,v) is
	std::unordered_map<CPUID, AffinityTask*> cpuToPrevTask;
	std::unordered_map<AffinityTask*, CPUID> taskToPrevCPU;
	bool reachable = false;

	std::queue<GraphNode> queue;//create a queue Q, true is Job, false is processor
	std::unordered_set<CPUID> visitedCPU; //create a set V
	std::unordered_set<AffinityTask*> visitedTask; //create a set V

	if(start.isTask())
		visitedTask.insert(start.getTask()); //add v to V
	else
		visitedCPU.insert(start.getCPUID());
	queue.push(start); //enqueue v onto Q

	while(!queue.empty())//while Q is not empty loop
	{
		auto currentItem = queue.front(); //t ← Q.dequeue()
		queue.pop();

		if(currentItem.isTask())
		{
			if(target.isTask())
				if(target.getTask() == currentItem.getTask()) //if t is what we are looking for then
				{
					//return t
					reachable = true;
					break;
				}
		}
		else
		{
			if(!target.isTask())
				if(target.getCPUID() == currentItem.getCPUID()) //if t is what we are looking for then
				{
					//return t
					reachable = true;
					break;
				}
		}

		//for all edges e in G.adjacentEdges(t) loop
		if(currentItem.isTask())
		{
			AffinityTask* curTask = currentItem.getTask();
			assert(curTask != nullptr);

			for(CPUID adjacentCPU : taskToCPUList.find(curTask)->second) //u ← G.adjacentVertex(t,e)
			{
				if(visitedCPU.find(adjacentCPU) == visitedCPU.end()) //if u is not in V then
				{
					visitedCPU.insert(adjacentCPU); //add u to V
					queue.push(GraphNode(adjacentCPU)); //enqueue u onto Q

					assert(cpuToPrevTask.find(adjacentCPU) == cpuToPrevTask.end());
					cpuToPrevTask.insert(std::pair<CPUID,AffinityTask*>(adjacentCPU, curTask));
				}
			}
		}
		else
		{
			CPUID curCPU = currentItem.getCPUID();
			auto iter = cpuToTaskList.find(curCPU);
			if(iter == cpuToTaskList.end())
			{
				continue;
			}
			assert(iter->second.size() > 0);
			for(AffinityTask* adjacentTask : iter->second) //u ← G.adjacentVertex(t,e)
			{
				if(visitedTask.find(adjacentTask) == visitedTask.end())
				{
					visitedTask.insert(adjacentTask);
					queue.push(GraphNode(adjacentTask));

					assert(taskToPrevCPU.find(adjacentTask) == taskToPrevCPU.end());
					taskToPrevCPU.insert(std::pair<AffinityTask*,CPUID>(adjacentTask, curCPU));
				}
			}
		}
	}

	if(reachable)
	{
		GraphNode current = target;
		while(true)
		{
			returnList.push_front(current);
			if(current.isTask())
			{
				auto cpu_iter = taskToPrevCPU.find(current.getTask());
				if(cpu_iter == taskToPrevCPU.end())
				{
					assert(start.isTask());
					assert(current.getTask() == start.getTask());
					break;
				}
				current = cpu_iter->second;
			}
			else
			{
				auto task_iter = cpuToPrevTask.find(current.getCPUID());
				if(task_iter == cpuToPrevTask.end())
				{
					assert(!start.isTask());
					assert(current.getCPUID() == start.getCPUID());
					break;
				}
				current = task_iter->second;
			}
		}
	}

	return returnList;
}

bool AffinityTask::staticWeakAnalysis(const TaskSet& taskSet, Time overhead)
{
	AffinityTask::Compare compare;
	std::unordered_map<AffinityTask*, Time> responseTime;
	for(AffinityTask* task : taskSet)
		responseTime.insert(std::pair<AffinityTask*, Time>(task, task->worstExecution));

	while(true)
	{
		bool changed = false;
		bool overflow = false;
		std::unordered_map<AffinityTask*, Time> newResponseTime;

		for(auto current : responseTime)
		{
			AffinityTask* curTask = current.first;
			std::set<Affinity> powerSet = AffinityTask::powerSet(curTask->affinity);
			Time currentResponse = responseTime.find(curTask)->second;

			Time min_sumInterfere = std::numeric_limits<Time>::max();
			for(Affinity s : powerSet)
			{
				assert(s.size() != 0);
				Size s_Size = s.size();
				Time sumInterference = 0;

				Time localSum = 0;
				TaskSet highPrioritySet;
				for(CPUID selectedCPU : s)
				{
					for(AffinityTask* highPriorityTask : taskSet)
					{
						if(compare(curTask, highPriorityTask))
							continue;
						if(highPriorityTask == curTask)
							continue;
						if(highPriorityTask->affinity.find(selectedCPU) == highPriorityTask->affinity.end())
							continue;

						highPrioritySet.insert(highPriorityTask);
					}
				}

				localSum += overhead;
				for(AffinityTask* highPriorityTask : highPrioritySet)
				{
					Time interferenceCount = currentResponse/highPriorityTask->minPeriod;
					Time remaining = currentResponse % highPriorityTask->minPeriod;
					Time interference = interferenceCount * highPriorityTask->worstExecution
							+ std::min(remaining, highPriorityTask->worstExecution);

					localSum += interference;
					Time contextSwitchCount = interferenceCount;
					if(remaining > 0)
						contextSwitchCount++;

					localSum += 2*(contextSwitchCount) * overhead;
				}

				sumInterference += localSum;


				Time floorValue = floor((Real)sumInterference / (Real)s_Size);

				min_sumInterfere = std::min(min_sumInterfere, floorValue);
			}
			assert(min_sumInterfere != std::numeric_limits<Time>::max());

			Time nextResponse = curTask->worstExecution + min_sumInterfere;
			newResponseTime.insert(std::pair<AffinityTask*, Time>(curTask, nextResponse));
			if(currentResponse != nextResponse)
				changed = true;
			if(currentResponse > curTask->minPeriod)
				overflow = true;
		}

		if(changed)
			responseTime = newResponseTime;
		else
			break;
		if(overflow)
			break;
	}

	bool possible = true;
	for(auto iter : responseTime)
	{
		if(iter.second > iter.first->minPeriod)
		{
			possible = false;
			iter.first->print_log(WARN, "Execution time: %lu, Period: %lu, Response time: %lu",
					iter.first->worstExecution, iter.first->minPeriod, iter.second);
		}
		else
		{
			iter.first->print_log(INFO, "Execution time: %lu, Period: %lu, Response time: %lu",
					iter.first->worstExecution, iter.first->minPeriod, iter.second);
		}
	}

	return possible;
}

void AffinityTask::combinePossibleTaskSet_recursive(const std::unordered_map<CPUID, std::list<TaskSet>>& possibleReplacement, const std::unordered_map<CPUID, CPUID>& nextCPU, const CPUID& currentCPU, const std::list<TaskSet>& visited, std::list<TaskSet>& saveAt)
{

	auto currentList = possibleReplacement.find(currentCPU);
	auto nextCPUIter = nextCPU.find(currentCPU);
	if(nextCPUIter == nextCPU.end())
	{
		//time to print
		for(auto currentItem : currentList->second)
		{
			TaskSet returnSet;
			for(auto curStack : visited)
			{
				for(auto task : curStack)
					returnSet.insert(task);
			}
			for(auto task : currentItem)
				returnSet.insert(task);
			saveAt.push_back(returnSet);
		}
	}
	else
	{
		auto nextCPUID = nextCPUIter->second;
		for(auto currentItem : currentList->second)
		{
			std::list<TaskSet> newVisited(visited);
			newVisited.push_back(currentItem);

			combinePossibleTaskSet_recursive(
					possibleReplacement, nextCPU, nextCPUID, newVisited, saveAt);
		}
	}
}

std::list<AffinityTask::TaskSet> AffinityTask::combinePossibleTaskSet(const std::unordered_map<CPUID, std::list<TaskSet>>& possibleReplacement)
{
	std::unordered_map<CPUID, CPUID> nextCPUIDMap;
	std::list<AffinityTask::TaskSet> returnValue;

	bool isFirst = true;
	CPUID prevCPU = 0;
	CPUID firstCPU = 0;
	for(auto iter : possibleReplacement)
	{
		assert(iter.second.size() > 0);
		if(!isFirst)
			nextCPUIDMap.insert(std::pair<CPUID, CPUID>(prevCPU, iter.first));
		else
			firstCPU = iter.first;

		isFirst = false;
		prevCPU = iter.first;
	}


	combinePossibleTaskSet_recursive(
			possibleReplacement, nextCPUIDMap, firstCPU, std::list<TaskSet>(), returnValue);

	return returnValue;
}


bool AffinityTask::staticStrongAnalysis(const TaskSet& taskSet, Time overhead)
{
	AffinityTask::Compare compare;
	std::unordered_map<AffinityTask*, Time> responseTime;
	Affinity allCPU;
	for(AffinityTask* task : taskSet)
	{
		responseTime.insert(std::pair<AffinityTask*, Time>(task, task->worstExecution));
		for(auto cpu : task->affinity)
			allCPU.insert(cpu);
	}

	while(true)
	{
		bool changed = false;
		bool overflow = false;
		std::unordered_map<AffinityTask*, Time> newResponseTime;

		for(auto current : responseTime)
		{
			AffinityTask* curTask = current.first;
			std::set<Affinity> powerSet = AffinityTask::powerSet(curTask->affinity);
			Time currentResponse = responseTime.find(curTask)->second;
			TaskSet ignoreTask;
			ignoreTask.insert(curTask);

			Time min_sumInterfere = std::numeric_limits<Time>::max();
			for(Affinity s : powerSet)
			{
				assert(s.size() != 0);
				Size s_Size = s.size();
				//Time sumInterference = 0;
				std::unordered_map<CPUID, std::list<TaskSet>> possibleReplacement;
				for(auto cpu : s)
				{
					possibleReplacement.insert(std::pair<CPUID, std::list<TaskSet>>
							(cpu, std::list<TaskSet>()));
				}

				for(CPUID selectedCPU : s)
				{
					Affinity ignoreCPU(s);
					ignoreCPU.erase(selectedCPU);
					for(auto alternative : allCPU)
					{
						if(ignoreCPU.find(alternative) != ignoreCPU.end())
							continue;
						auto allPaths = allPath(taskSet, selectedCPU, alternative, ignoreCPU, ignoreTask);
						for(auto path : allPaths)
						{
							if(path.size() > 0)
							{
								TaskSet ignoredTask;
								Affinity moreCheck;
								for(auto item : path)
								{
									if(item.isTask())
										ignoredTask.insert(item.getTask());
									else
										moreCheck.insert(item.getCPUID());
								}
								TaskSet highTaskSet;
								for(AffinityTask* highPriorityTask : taskSet)
								{
									//if(compare(curTask, highPriorityTask))
									//	continue;
									if(highPriorityTask == curTask)
										continue;
									if(ignoredTask.find(highPriorityTask) != ignoredTask.end())
										continue;

									bool intersect = false;
									for(auto cpu : highPriorityTask->affinity)
									{
										if(moreCheck.find(cpu) != moreCheck.end())
										{
											intersect = true;
											break;
										}
									}
									if(!intersect)
										continue;

									highTaskSet.insert(highPriorityTask);
								}
								possibleReplacement.find(selectedCPU)->second.push_back(highTaskSet);
							}
						}
					}
				}


				for(auto possibleSet : combinePossibleTaskSet(possibleReplacement))
				{
					Time sumInterference = 0;
					/*
					if(possibleSet.size() ==0)
						continue;

					assert(possibleSet.size() > 0);
					*/
					sumInterference += overhead;
					for(auto highPriorityTask : possibleSet)
					{

						Time interferenceCount = currentResponse/highPriorityTask->minPeriod;
						Time remaining = currentResponse % highPriorityTask->minPeriod;
						Time interference = interferenceCount * highPriorityTask->worstExecution
								+ std::min(remaining, highPriorityTask->worstExecution);

						Time contextSwitchCount = interferenceCount;
						if(remaining > 0)
							contextSwitchCount++;

						sumInterference += 2*(contextSwitchCount) * overhead;

						if(compare(curTask, highPriorityTask))
							continue;

						sumInterference += interference;
					}


					Time floorValue = floor((Real)sumInterference / (Real)s_Size);

					min_sumInterfere = std::min(min_sumInterfere, floorValue);
				}
			}
			assert(min_sumInterfere != std::numeric_limits<Time>::max());

			Time nextResponse = curTask->worstExecution + min_sumInterfere;
			newResponseTime.insert(std::pair<AffinityTask*, Time>(curTask, nextResponse));
			if(currentResponse != nextResponse)
				changed = true;
			if(currentResponse > curTask->minPeriod)
				overflow = true;
		}

		if(changed)
			responseTime = newResponseTime;
		else
			break;
		if(overflow)
			break;
	}

	bool possible = true;
	for(auto iter : responseTime)
	{
		if(iter.second > iter.first->minPeriod)
		{
			possible = false;
			iter.first->print_log(WARN, "Execution time: %lu, Period: %lu, Response time: %lu",
					iter.first->worstExecution, iter.first->minPeriod, iter.second);
		}
		else
		{
			iter.first->print_log(INFO, "Execution time: %lu, Period: %lu, Response time: %lu",
					iter.first->worstExecution, iter.first->minPeriod, iter.second);
		}
	}

	return possible;
}

AffinityTask::TaskSet AffinityTask::copyTaskSet(Computer* computer, const AffinityTask::TaskSet& orig)
{
	AffinityTask::TaskSet ret;
	for(auto item : orig)
	{
		AffinityTask* task = new AffinityTask(*item, computer);
		ret.insert(task);
	}
	return ret;
}

AffinityTask::TaskSet AffinityTask::generateTaskSet(Computer* computer, Size numTask,
		RandomDistribution* affinityDistribution,
		Time minPeriod, Time maxPeriod, RandomDistribution* periodDistribution,
		Real targetUtilization, RandomDistribution* utilDistribution)
{
	AffinityTask::TaskSet ret;
	CPUID maxAffinity = computer->getNumCPU();
	UniformDistribution selector;
	Real sum = 0;
	for(Real util : utilDistribution->distribute(numTask, targetUtilization))
	{
		util = std::min(1.0, util);
		Real realAffinity = affinityDistribution->nextDistribution(0, maxAffinity);
		Size currentAffinityCount = ceil(realAffinity);
		currentAffinityCount = std::min(currentAffinityCount, maxAffinity);
		currentAffinityCount = std::max(currentAffinityCount, (Size)1);

		Real realPeriod = periodDistribution->nextDistribution(minPeriod, maxPeriod);
		Time period = ceil(realPeriod);
		period = std::max(minPeriod, period);
		period = std::min(maxPeriod, period);

		Real realExecution = util * (Real)period;
		Time execution = floor(realExecution);
		if(execution == 0)
			continue;

		sum+=util;

		Affinity affinity;

		while(affinity.size() != currentAffinityCount)
		{
			CPUID selected = floor(selector.nextDistribution(0, maxAffinity));
			if(selected == maxAffinity)
				selected = maxAffinity-1;
			affinity.insert(selected);
		}

		AffinityTask* task = new AffinityTask(affinity, computer, period, execution, 0);
		ret.insert(task);
	}
	//printf("Sum of util %f\n", sum);
	return ret;

}

void AffinityTask::cleanTaskSet(TaskSet& taskSet)
{
	for(AffinityTask* task : taskSet)
		delete task;
	taskSet.clear();
}


}
