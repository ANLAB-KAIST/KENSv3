/*
 * E_System.cpp
 *
 *  Created on: 2014. 11. 1.
 *      Author: Keunhong Lee
 */

#include <E/E_System.hpp>
#include <E/E_Module.hpp>

namespace E
{
class Module;

System::System()
{
	currentTime = 0;
	this->currentID = 0;
}

System::~System()
{
	activeTimer.clear();
	while(!timerQueue.empty())
	{
		TimerContainer* current = timerQueue.top();
		timerQueue.pop();
		delete current;
	}
}

UUID System::sendMessage(Module* from, Module* to, Module::Message* message, Time timeAfter)
{
	UUID uuid = allocateUUID();
	TimerContainer* container = new TimerContainer;
	container->from = from;
	container->to = to;
	container->canceled = false;
	container->wakeup = this->getCurrentTime() + timeAfter;
	container->message = message;
	container->uuid = uuid;

	activeTimer.insert(std::pair<UUID, TimerContainer*>(uuid, container));
	timerQueue.push(container);

	return uuid;
}
UUID System::allocateUUID()
{
	UUID startID = currentID;
	do
	{
		UUID candidate = currentID++;
		if(activeUUID.find(candidate) == activeUUID.end())
		{
			activeUUID.insert(candidate);
			return candidate;
		}
	}while(startID != currentID);
	assert(0);
	return 0;
}

bool System::deallocateUUID(UUID candidate)
{
	if(activeUUID.find(candidate) == activeUUID.end())
		return false;
	activeUUID.erase(candidate);
	return true;
}

void System::registerModule(Module* module)
{
	this->registeredModule.insert(module);
}
void System::unregisterModule(Module* module)
{
	this->registeredModule.erase(module);
}
bool System::isRegistered(Module* module)
{
	return this->registeredModule.find(module) != this->registeredModule.end();
}

Time System::getCurrentTime()
{
	return this->currentTime;
}

bool System::cancelMessage(UUID messageID)
{
	std::unordered_map<UUID, TimerContainer*>::iterator iter = this->activeTimer.find(messageID);
	if(iter == this->activeTimer.end())
		return false;
	iter->second->canceled = true;
	return true;
}

void System::run(Time till)
{
	std::vector<TimerContainer*> sameTime;
	std::unique_lock<std::mutex> lock(this->getSystemLock(), std::defer_lock);

	lock.lock();
	while(true)
	{
		while(true)
		{
			bool found = false;
			for(Runnable* r : runnableSet)
			{
				if(r->isRunning())
				{
					r->waitForRunning(false, lock);
					found = true;
					break;
				}
			}
			if(!found)
				break;
		}
		if(timerQueue.empty())
		{
			/*
			if(!runnableSet.empty())
			{
				lock.unlock();
				std::this_thread::yield();
				lock.lock();
				continue;
			}
			else
			{
				lock.unlock();
				std::this_thread::yield();
				lock.lock();
				break;
			}
			*/
			break;
		}
		if(till != 0 && timerQueue.top()->wakeup > till)
			break;

		TimerContainer* current = timerQueue.top();
		assert(current);
		timerQueue.pop();
#if 0
		TimerContainer* temp;
		sameTime.clear();
		while(true)
		{
			if(timerQueue.empty())
				break;
			temp = timerQueue.top();

			if(temp->wakeup == current->wakeup)
			{
				timerQueue.pop();
				sameTime.push_back(temp);
			}
			else
				break;
		}
		assert(sameTime.size() > 0);
		if(sameTime.size() != 1)
		{
			print_log(INFO, "Multiple (%lu) modules wake up on %lu, shuffle.", sameTime.size(), current->wakeup);

			std::random_shuffle(sameTime.begin(), sameTime.end());
		}
#endif
		this->currentTime = current->wakeup;
		//for(TimerContainer* container : sameTime)
		{
			TimerContainer* container = current;
			if(!container->canceled)
			{
				Module::Message* ret = container->to->messageReceived(container->from, container->message);
				container->from->messageFinished(container->to, container->message, ret);
				if(ret != nullptr)
					container->to->messageFinished(container->to, ret, nullptr);
			}
			else
			{
				container->from->messageCancelled(container->to, container->message);
			}

			this->activeTimer.erase(container->uuid);
			this->activeUUID.erase(container->uuid);
			delete container;
		}
	}
	lock.unlock();
}

std::mutex& System::getSystemLock()
{
	return this->mutex;
}

Runnable::Runnable(System* system, bool initial_value) : running(initial_value)
{
	this->system = system;
}
Runnable::~Runnable()
{

}

void Runnable::setRunning(bool value)
{
	//system->getSystemLock().lock()
	//assert(system->getSystemLock().owns_lock());
	this->running = value;
	cond.notify_all();
}

bool Runnable::isRunning()
{
	//assert(system->getSystemLock().owns_lock());
	bool ret = this->running;
	return ret;
}

void Runnable::waitForRunning(bool value, std::unique_lock<std::mutex>& lock)
{
	//assert(system->getSystemLock().owns_lock());
	while(this->running != value)
		cond.wait(lock);
}

void System::addRunnable(Runnable* runnable)
{
	//assert(this->getSystemLock().owns_lock());
	this->runnableSet.insert(runnable);
}
void System::delRunnable(Runnable* runnable)
{
	//assert(this->getSystemLock().owns_lock());
	this->runnableSet.erase(runnable);
}

}
