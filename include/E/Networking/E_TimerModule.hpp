/**
 * @file   E_TimerModule.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::TimerModule
 */

#ifndef E_HOST_TIMERMODULE_HPP_
#define E_HOST_TIMERMODULE_HPP_
#include <E/E_Common.hpp>

namespace E {

class Host;

/**
 * @brief TimerModule provides convenient interface of timer/alarm.
 * @note If you need custom Message handling, DO NOT USE THIS CLASS.
 * i.e. you cannot use both Module and TimerModule
 *
 * @see Module
 */
class TimerModule {
private:
  Host &host;
  std::string name;

protected:
  TimerModule(std::string name, Host &host);
  virtual ~TimerModule();

  /**
   * @return My name used in the registered Host.
   */
  virtual std::string getTimerModuleName() final;

  /**
   * @brief This function is automatically when requested alarm is triggered.
   * You must override this pure virtual function to use TimerModule.
   *
   * @param payload Metadata that you specified via addTimer
   *
   * @note TimerModule does nothing for the payload. You may deallocate payload
   * IF NEEDED.
   */
  virtual void timerCallback(std::any payload) = 0;

  /**
   * @brief Request an alarm that rings after specified time.
   *
   * @param payload Metadata you needed. Can be null.
   * @param timeAfter Specify when the alarm will ring.
   * @return Unique ID that indicates the timer request.
   *
   * @note You cannot override this function.
   */
  virtual UUID addTimer(std::any payload, Time timeAfter) final;

  /**
   * @brief Cancel the timer request.
   *
   * @param key Unique ID that indicates the timer request.
   *
   * @note You cannot override this function.
   * There is no Module::messageCancelled here, so be sure that
   * you deallocated any resources you allocated for the timer.
   *
   * @see addTimer, Module::messageCancelled
   */
  virtual void cancelTimer(UUID key) final;

  friend class Host;
};

} // namespace E

#endif /* E_HOST_TIMERMODULE_HPP_ */
