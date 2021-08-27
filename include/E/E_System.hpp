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

namespace E {

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
class System : private Log {
private:
  std::unordered_set<std::shared_ptr<Runnable>> runnableReady;
  static const ModuleID newModuleID();
  class TimerContainerInner {
  public:
    const ModuleID from;
    const ModuleID to;
    bool canceled;
    Time wakeup;
    Module::Message message;
    UUID uuid;
    TimerContainerInner(const ModuleID from, const ModuleID to, bool canceled,
                        Time wakeup, Module::Message message, UUID uuid)
        : from(from), to(to), canceled(canceled), wakeup(wakeup),
          message(std::move(message)), uuid(uuid) {}
  };

  using TimerContainer = std::shared_ptr<TimerContainerInner>;

  class TimerContainerLess {
  public:
    bool operator()(const TimerContainer a, const TimerContainer b) {
      if (a->wakeup != b->wakeup)
        return a->wakeup > b->wakeup;
      else
        return a->uuid > b->uuid; // XXX if uuid returns to the start??
    }
  };
  UUID currentID;
  Time currentTime;

protected:
  ModuleID lookupModuleID(Module &module);
  std::unordered_map<ModuleID, std::shared_ptr<Module>> registeredModule;

private:
  std::priority_queue<TimerContainer, std::vector<TimerContainer>,
                      TimerContainerLess>
      timerQueue;
  std::unordered_map<UUID, TimerContainer> activeTimer;
  std::unordered_set<UUID> activeUUID;

  UUID allocateUUID();
  bool deallocateUUID(UUID uuid);
  bool isRegistered(const ModuleID moduleID);
  UUID sendMessage(const ModuleID from, const ModuleID to,
                   Module::Message message, Time timeAfter);
  bool cancelMessage(UUID messageID);

public:
  /**
   * @brief Nothing is needed to construct a System
   */
  System();
  virtual ~System();

  /**
   * @brief Execute all registered Module. Virtual clock will move to the end of
   * the simulation.
   * @param till The System will operate until the given virtual clock.
   * If all events are finished, it will immediately return even if it did not
   * reached to the given termination time.
   */
  void run(Time till);

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
  virtual void addRunnable(std::shared_ptr<Runnable> runnable) final;

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
  virtual void delRunnable(std::shared_ptr<Runnable> runnable) final;

  template <typename T, typename... Args>
  std::shared_ptr<T> addModule(Args &&...args) {
    static_assert(std::is_base_of<Module, T>::value);
    auto module = std::make_shared<T>(std::forward<Args>(args)...);
    module->id = newModuleID();
    bool ret = registeredModule.insert({module->id, module}).second;
    (void)ret;
    assert(ret);
    return module;
  }
  std::string getModuleName(const ModuleID moduleID);

  friend Module::Module(System &system);
  friend Module::~Module();

  friend UUID Module::sendMessage(const ModuleID to, Module::Message message,
                                  Time timeAfter);
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
class Runnable {
protected:
  /**
   * @brief Constructs a Runnable interface.
   */
  Runnable();
  virtual ~Runnable();

  /**
   * @brief Enter wait state (RUNNING -> WAITING)
   */
  virtual void wait() final;
  /**
   * @brief Prepare logic
   */
  virtual void pre_main(){};

  /**
   * @brief Program logic
   */
  virtual void main() = 0;

  /**
   * @brief Thread code
   */
  virtual void run() final;

public:
  enum class State {
    CREATED,
    STARTING,
    READY,
    RUNNING,
    WAITING,
    TERMINATED,
  };
  /**
   * @brief Wake and keep running (CREATED -> READY)
   */
  virtual void start() final;
  /**
   * @brief Wake and keep running (READY -> RUNNING)
   *
   * @return next state
   *
   * @note Scheduler will be blocked
   */
  virtual State wake() final;

  /**
   * @brief Mark ready (WAITING -> READY)
   */
  virtual void ready() final;
  friend class System;

private:
  State state;
  std::mutex stateMtx;
  std::unique_lock<std::mutex> threadLock; //  for thread
  std::unique_lock<std::mutex> schedLock;  //  for scheduler
  std::condition_variable cond;
  std::thread thread;
};

} // namespace E

#endif /* E_SYSTEM_HPP_ */
