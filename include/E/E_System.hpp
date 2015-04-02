/**
 * @file   E_System.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::System and E::Runnable
 */

#ifndef E_SYSTEM_HPP_
#define E_SYSTEM_HPP_

#include <E/E_Common.hpp>
#include <E/E_Log.hpp>
#include <E/E_Module.hpp>

namespace E
{

class TimerContainer;
class Runnable;


/**
 * @brief System provides a virtual clock used during the simulation.
 * Current virtual clock can be obtained by System::getCurrentTime.
 * Every Module registered in the System share same virtual clock
 * and the System manages the total ordering of every event.
 *
 * @see System::getCurrentTime
 * @see Module
 */
class System : private Log
{
private:
	std::unordered_set<Runnable*> runnableSet;
	std::mutex mutex;
	class TimerContainer
	{
	public:
		Module* from;
		Module* to;
		bool canceled;
		Time wakeup;
		Module::Message* message;
		UUID uuid;
	};

	class TimerContainerLess
	{
	public:
		bool operator()(const TimerContainer* a , const TimerContainer* b )
		{
			if(a->wakeup != b->wakeup)
				return a->wakeup > b->wakeup;
			else
				return a->uuid > b->uuid; //XXX if uuid returns to the start??
		}
	};
	UUID currentID;
	Time currentTime;
	std::priority_queue<TimerContainer*, std::vector<TimerContainer*>, TimerContainerLess> timerQueue;
	std::unordered_map<UUID, TimerContainer*> activeTimer;
	std::unordered_set<UUID> activeUUID;
	std::unordered_set<Module*> registeredModule;

	UUID allocateUUID();
	bool deallocateUUID(UUID uuid);
	void registerModule(Module* module);
	void unregisterModule(Module* module);
	bool isRegistered(Module* module);
	UUID sendMessage(Module* from, Module* to, Module::Message* message, Time timeAfter);
	bool cancelMessage(UUID messageID);

public:
	/**
	 * @brief Nothing is needed to construct a System
	 */
	System();
	virtual ~System();

	/**
	 * @brief Execute all registered Module. Virtual clock will move to the end of the simulation.
	 * @param till The System will operate until the given virtual clock.
	 * If all events are finished, it will immediately return even if it did not reached to the given termination time.
	 */
	void run(Time till);

	/**
	 * @return Global lock object used in this System
	 */
	std::mutex& getSystemLock();

	/**
	 * @return Returns current virtual clock of the System.
	 */
	Time getCurrentTime();

	/**
	 * @brief Register a Runnable interface to this System.
	 *
	 * @param runnable Runnable interface to be added.
	 *
	 * @return nothing
	 *
	 * @note You cannot override this function.
	 */
	virtual void addRunnable(Runnable* runnable) final;

	/**
	 * @brief Unregister a Runnable interface to this System.
	 *
	 * @param runnable Runnable interface to be removed.
	 * The interface must be registered before being removed.
	 *
	 * @return nothing
	 *
	 * @note You cannot override this function.
	 */
	virtual void delRunnable(Runnable* runnable) final;

	friend Module::Module(System* system);
	friend Module::~Module();

	friend UUID Module::sendMessage(Module* to, Module::Message* message, Time timeAfter);
	friend bool Module::cancelMessage(UUID timer);
};

/**
 * @brief Runnable is executed via System.
 * The total ordering of execution is guaranteed by the System.
 *
 * @note You cannot instantiate this class.
 * You must inherit this class to use.
 *
 * @see System
 */
class Runnable
{
protected:
	/**
	 * @brief Constructs a Runnable interface.
	 *
	 * @param system System to be registered.
	 * Runnable is automatically registered when it is created.
	 * @param initial_value Initial running state (true by default).
	 */
	Runnable(System* system, bool initial_value = true);
	virtual ~Runnable();

	/**
	 * @param value Set the running state
	 * @note The System would not terminate unless all Runnable becomes "stopped".
	 * You cannot override this function.
	 */
	virtual void setRunning(bool value) final;

public:
	/**
	 * @return Current running state.
	 * @note You cannot override this function.
	 */
	virtual bool isRunning() final;

	/**
	 * @brief Wait until current running state becomes the given value.
	 * @param value Value you are waiting for.
	 * @param lock Global lock of the system.
	 *
	 * @note You cannot override this function.
	 * @see System::getSystemLock
	 */
	virtual void waitForRunning(bool value, std::unique_lock<std::mutex>& lock) final;

private:
	System* system;
	bool running;
	std::condition_variable cond;
};

}

#endif /* E_SYSTEM_HPP_ */
