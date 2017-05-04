/*
 * E_Job.cpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */

#include <E/E_System.hpp>
#include <E/Scheduling/E_Computer.hpp>
#include <E/Scheduling/E_Job.hpp>
#include <E/Scheduling/E_Processor.hpp>

namespace E
{



Job::Job(Time raisedAt, Time executionTime, Time deadLine, Task* task)
{
	this->raisedAt = raisedAt;
	this->executionTime = executionTime;
	this->deadLine = deadLine;
	this->remaining = this->executionTime;
	this->task = task;
	this->isChecking = false;
	this->checkMessage = 0;
	this->processor = nullptr;
}
Job::~Job()
{

}

void Job::execute(Time run)
{
	if(run > remaining)
		assert(run <= remaining);
	this->remaining -= run;
}

bool Job::isDone() const
{
	return this->remaining == 0;
}

Time Job::getRaisedTime() const
{
	return this->raisedAt;
}

Time Job::getDeadLine() const
{
	return this->deadLine;
}
Time Job::getExecutionTime() const
{
	return this->executionTime;
}
Time Job::getRemaining() const
{
	return this->remaining;
}

Task* Job::getTask() const
{
	return this->task;
}


}


