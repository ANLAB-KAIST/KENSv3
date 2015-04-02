/*
 * E_Affinity.hpp
 *
 *  Created on: Nov 4, 2014
 *      Author: leeopop
 */

#ifndef E_AFFINITY_HPP_
#define E_AFFINITY_HPP_

#include <E/E_Common.hpp>
#include <E/E_RandomDistribution.hpp>
#include <E/Scheduling/E_Task.hpp>
#include <E/Scheduling/RM/E_RMScheduler.hpp>

namespace E
{

typedef std::set<CPUID> Affinity;
class AffinityTask;
class GraphNode
{
public:
	GraphNode(AffinityTask* task);
	GraphNode(CPUID cpu);
	bool isTask() const;
	~GraphNode();
	AffinityTask* getTask() const;
	CPUID getCPUID() const;
private:
	bool type; // true is task
	union
	{
		AffinityTask* task;
		CPUID cpu;
	};
};

class AffinityTask : public SporadicTask, private Log
{
protected:
	Affinity affinity;

public:
	AffinityTask(Affinity affinity, Computer* computer, Time period, Time executionTime, Time startOffset);
	AffinityTask(const AffinityTask& orig, Computer* computer);
	virtual ~AffinityTask();

	const Affinity& getAffinity();

	typedef std::set<AffinityTask*> TaskSet;

	static std::set<Affinity> powerSet(const Affinity& affinity);
	static bool staticStrongAnalysis(const TaskSet& taskSet, Time overhead = 0);

	static std::list<GraphNode> BFS(const TaskSet& taskSet, const GraphNode& start, const GraphNode& target, const Affinity& excludeID, const TaskSet& excludeTask);
	static void DFS(std::list<std::list<GraphNode>>& saveAt, const std::unordered_map<CPUID, TaskSet>& cpuToTaskList,
			const std::unordered_map<AffinityTask*, Affinity>& taskToCPUList,
			const GraphNode& start, const GraphNode& target, const std::list<GraphNode>& visited);
	static std::list<std::list<GraphNode>> allPath(const TaskSet& taskSet, const GraphNode& start, const GraphNode& target, const Affinity& excludeID, const TaskSet& excludeTask);
	static bool staticWeakAnalysis(const TaskSet& taskSet, Time overhead = 0);
	static std::list<TaskSet> combinePossibleTaskSet(const std::unordered_map<CPUID, std::list<TaskSet>>& possibleReplacement);
	static void combinePossibleTaskSet_recursive(const std::unordered_map<CPUID, std::list<TaskSet>>& possibleReplacement, const std::unordered_map<CPUID, CPUID>& nextCPU, const CPUID& currentCPU, const std::list<TaskSet>& visited, std::list<TaskSet>& saveAt);

	static AffinityTask::TaskSet copyTaskSet(Computer* computer, const AffinityTask::TaskSet& orig);
	static TaskSet generateTaskSet(Computer* computer, Size numTask,
			RandomDistribution* affinityDistribution,
			Time minPeriod, Time maxPeriod, RandomDistribution* periodDistribution,
			Real targetUtilization, RandomDistribution* utilDistribution);
	static void cleanTaskSet(TaskSet& taskSet);

	typedef RMJobCompare JobCompare;
public:
	class Compare
	{
	public:
		bool operator()(const Task* _a , const Task* _b)
		{
			const AffinityTask* a = dynamic_cast<const AffinityTask*>(_a);
			const AffinityTask* b = dynamic_cast<const AffinityTask*>(_b);
			assert(a != nullptr);
			assert(b != nullptr);
			return a->minPeriod < b->minPeriod;
		}
	};
};



}

#endif /* E_AFFINITY_HPP_ */
