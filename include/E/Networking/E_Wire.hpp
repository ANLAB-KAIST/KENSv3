/**
 * @file   E_Wire.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::Wire
 */

#ifndef E_WIRE_HPP_
#define E_WIRE_HPP_

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/Networking/E_NetworkLog.hpp>
#include <E/Networking/E_Packet.hpp>

namespace E {
class NetworkSystem;

/**
 * @brief Wire does a role of 2-ended wire.
 * However there is a speed limit and delayed packets are queued
 * (currently, no limitation to the queue length).
 */
class Wire : public Module, protected NetworkLog {
private:
  std::array<ModuleID, 2> connected;
  std::array<Time, 2> nextAvailable;
  Time propagationDelay;
  Size bps;
  bool limit_speed;

public:
  /**
   * @brief Create a Wire.
   * @param name See NetworkModule
   * @param system See NetworkSysetm
   * @param left Module ID for left
   * @param right Module ID for right
   * @param propagationDelay Propagation delay in nanoseconds.
   * @param bps Link speed in bit per sec.
   * @param limit_speed Control speed limiting functionality.
   */
  Wire(std::string name, NetworkSystem &system, ModuleID left, ModuleID right,
       Time propagationDelay = 1000000, Size bps = 1000000000UL,
       bool limit_speed = true);
  virtual ~Wire();

  /**
   * @param do_limit Control speed limiting functionality.
   * @note You cannot override this function.
   */
  virtual void setSpeedLimit(bool do_limit) final;

  /**
   * @param bps Set data rate.
   * @note You cannot override this function.
   */
  virtual void setWireSpeed(Size bps) final;

  /**
   * @return Get data rate.
   * @note You cannot override this function.
   */
  virtual Size getWireSpeed() final;

  /**
   * @param delay Set propagation delay.
   * @note You cannot override this function.
   */
  virtual void setPropagationDelay(Time delay) final;

  enum MessageType {
    PACKET_TO_PORT,
    PACKET_FROM_PORT,
  };

  class Message : public Module::MessageBase {
  public:
    enum MessageType type;
    Packet packet;

    Message(enum MessageType type, Packet &&packet)
        : type(type), packet(packet) {}

    ~Message() override = default;
  };

  virtual Time nextSendAvailable(const ModuleID me) final;

private:
  virtual Module::Message messageReceived(const ModuleID from,
                                          Module::MessageBase &message) final;
  virtual void messageFinished(const ModuleID to, Module::Message message,
                               Module::MessageBase &response) final;
  virtual void messageCancelled(const ModuleID to,
                                Module::Message message) final;
};

} // namespace E

#endif /* E_WIRE_HPP_ */
