/*
 * E_Scheduler.cpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */

#include <E/Scheduling/E_Computer.hpp>
#include <E/Scheduling/E_Job.hpp>
#include <E/Scheduling/E_Scheduler.hpp>

namespace E
{


Scheduler::Scheduler()
{
	this->computer = nullptr;
}
Scheduler::~Scheduler()
{

}

void Scheduler::setTimer(Time time, void* arg)
{
	this->computer->setTimer(time, arg);
}
void Scheduler::cancelTimer()
{
	this->computer->cancelTimer();
}

}
