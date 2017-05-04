/*
 * E_Job.hpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */

#ifndef E_JOB_HPP_
#define E_JOB_HPP_

#include <E/E_Common.hpp>
#include <E/Scheduling/E_Computer.hpp>

namespace E
{

class Task;
class Processor;
class Computer;

class Job
{
private:
	Time raisedAt;
	Time executionTime;
	Time deadLine;
	Time remaining;
	Task* task;
	UUID checkMessage;
	bool isChecking;
	Processor* processor;

	Job(Time raisedAt, Time executionTime, Time deadLine, Task* task);
	virtual ~Job();

	friend Computer;
	friend Processor;
public:
	bool isDone() const;

	void execute(Time run);

	Time getRaisedTime() const;
	Time getDeadLine() const;
	Time getExecutionTime() const;
	Time getRemaining() const;
	Task* getTask() const;
};

}


#endif /* E_JOB_HPP_ */
