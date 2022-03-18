/*
 * E_System.cpp
 *
 *  Created on: 2014. 11. 1.
 *      Author: Keunhong Lee
 */

#include <E/E_Module.hpp>
#include <E/E_System.hpp>

namespace E {
class Module;

System::System() {
  currentTime = 0;
  this->currentID = 0;
}

System::~System() {
  activeTimer.clear();
  while (!timerQueue.empty()) {
    timerQueue.pop();
  }

  for (auto it = registeredModule.begin(); it != registeredModule.end(); ++it) {

    if (it->second.use_count() != 1) {
      printf("Module must not live longer than System\n");
      abort();
    }
  }
}
UUID System::sendMessage(const ModuleID from, const ModuleID to,
                         Module::Message message, Time timeAfter) {
  UUID uuid = allocateUUID();
  TimerContainer container = std::make_shared<TimerContainerInner>(
      from, to, false, this->getCurrentTime() + timeAfter, std::move(message),
      uuid);

  activeTimer.insert(std::pair<UUID, TimerContainer>(uuid, container));
  timerQueue.push(container);

  return uuid;
}
UUID System::allocateUUID() {
  UUID startID = currentID;
  do {
    UUID candidate = currentID++;
    if (activeUUID.find(candidate) == activeUUID.end()) {
      activeUUID.insert(candidate);
      return candidate;
    }
  } while (startID != currentID);
  assert(0);
  return 0;
}

bool System::deallocateUUID(UUID candidate) {
  if (activeUUID.find(candidate) == activeUUID.end())
    return false;
  activeUUID.erase(candidate);
  return true;
}

bool System::isRegistered(const ModuleID module) {
  return this->registeredModule.find(module) != this->registeredModule.end();
}

Time System::getCurrentTime() { return this->currentTime; }

bool System::cancelMessage(UUID messageID) {
  std::unordered_map<UUID, TimerContainer>::iterator iter =
      this->activeTimer.find(messageID);
  if (iter == this->activeTimer.end())
    return false;
  iter->second->canceled = true;
  return true;
}

void System::run(Time till) {
  std::vector<TimerContainer> sameTime;

  while (true) {
    while (!runnableReady.empty()) {
      for (auto r = runnableReady.begin(); r != runnableReady.end();) {
        auto next = (*r)->wake();
        if (next != Runnable::State::READY) {
          r = runnableReady.erase(r);
        } else {
          ++r;
        }
      }
    }
    if (timerQueue.empty()) {
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
    if (till != 0 && timerQueue.top()->wakeup > till)
      break;

    TimerContainer current = timerQueue.top();
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
    // for(TimerContainer* container : sameTime)
    {
      TimerContainer container = std::move(current);
      if (!container->canceled) {
        Module::Message ret = registeredModule[container->to]->messageReceived(
            container->from, *container->message);
        registeredModule[container->from]->messageFinished(
            container->to, std::move(container->message),
            ret != nullptr ? *ret : Module::EmptyMessage::shared());
        if (ret != nullptr)
          registeredModule[container->to]->messageFinished(
              container->to, std::move(ret), Module::EmptyMessage::shared());
      } else {
        registeredModule[container->from]->messageCancelled(
            container->to, std::move(container->message));
      }

      this->activeTimer.erase(container->uuid);
      this->activeUUID.erase(container->uuid);
      assert(container.use_count() == 1);
    }
  }
}

Runnable::Runnable()
    : state(State::CREATED), threadLock(stateMtx, std::defer_lock),
      schedLock(stateMtx), thread(&Runnable::run, this) {}
Runnable::~Runnable() {
  assert(schedLock.owns_lock());
  assert(std::this_thread::get_id() != thread.get_id());
  thread.join();
}

void Runnable::run() {
  threadLock.lock();
  assert(std::this_thread::get_id() == thread.get_id());
  cond.wait(threadLock, [&] { return state == State::STARTING; });
  pre_main();
  state = State::READY;
  cond.notify_all();
  cond.wait(threadLock, [&] { return state == State::RUNNING; });
  main();
  state = State::TERMINATED;
  cond.notify_all();
  threadLock.unlock();
}

void Runnable::wait() {
  assert(threadLock.owns_lock());
  assert(std::this_thread::get_id() == thread.get_id());
  assert(state == State::RUNNING);
  state = State::WAITING;
  cond.notify_all();
  cond.wait(threadLock, [&] { return state == State::RUNNING; });
}

void Runnable::start() {
  assert(schedLock.owns_lock());
  assert(std::this_thread::get_id() != thread.get_id());
  assert(state == State::CREATED);
  state = State::STARTING;
  cond.notify_all();
  cond.wait(schedLock, [&] { return state == State::READY; });
  assert(schedLock.owns_lock());
}
Runnable::State Runnable::wake() {
  assert(schedLock.owns_lock());
  assert(std::this_thread::get_id() != thread.get_id());
  assert(state == State::READY);
  state = State::RUNNING;
  cond.notify_all();
  cond.wait(schedLock, [&] { return state != State::RUNNING; });
  return state;
}
void Runnable::ready() {
  assert(schedLock.owns_lock());
  assert(std::this_thread::get_id() != thread.get_id());
  assert(state == State::WAITING);
  state = State::READY;
}

void System::addRunnable(std::shared_ptr<Runnable> runnable) {
  assert(runnable->state == Runnable::State::READY);
  assert(runnable->schedLock.owns_lock());
  this->runnableReady.insert(runnable);
}
void System::delRunnable(std::shared_ptr<Runnable> runnable) {
  this->runnableReady.erase(runnable);
}

std::string System::getModuleName(const ModuleID moduleID) {

  auto it = registeredModule.find(moduleID);
  if (it != registeredModule.end()) {
    return it->second->getModuleName();
  } else {
    return "Nill";
  }
}
const ModuleID System::newModuleID() {
  static ModuleID next = 1;
  return next++;
}

ModuleID System::lookupModuleID(Module &module) { return module.id; }

} // namespace E
