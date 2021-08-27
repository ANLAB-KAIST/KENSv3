/**
 * @file   E_Link.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::Link
 */

#ifndef E_LINK_HPP_
#define E_LINK_HPP_

#include <E/E_Common.hpp>
#include <E/E_RandomDistribution.hpp>
#include <E/Networking/E_NetworkLog.hpp>
#include <E/Networking/E_Networking.hpp>
#include <E/Networking/E_Wire.hpp>
#include <fstream>

namespace E {
class Packet;

/**
 * @brief Link makes connections among multiple Wires.
 * It supports packet switching and output queuing.
 * Random drop occurs when the queue is full.
 */
class Link : public NetworkModule, private NetworkLog {

private:
  virtual Module::Message messageReceived(const ModuleID from,
                                          Module::MessageBase &message) final;
  virtual void messageFinished(const ModuleID to, Module::Message message,
                               Module::MessageBase &response) final;
  virtual void messageCancelled(const ModuleID to,
                                Module::Message message) final;

  std::ofstream pcap_file;
  bool pcap_enabled;
  Size snaplen;
  LinearDistribution rand_dist;

protected:
  std::unordered_map<ModuleID, Time> nextAvailable;
  std::unordered_map<ModuleID, std::list<Packet>> outputQueue;
  Size bps;
  Size max_queue_length;
  virtual void packetArrived(const ModuleID inWireID, Packet &&packet) = 0;
  virtual void packetSent(const ModuleID wireID, Packet &&packet) {
    (void)wireID;
    (void)packet;
  };
  virtual void sendPacket(const ModuleID wireID, Packet &&packet) final;

public:
  Link(std::string name, NetworkSystem &system);
  virtual ~Link();

  /**
   * @brief Make a PCAP formatted log file.
   * @param filename Name of the log file.
   * @param snaplen Length of packet data to be recorded.
   * @note You cannot override this function.
   */
  virtual void enablePCAPLogging(const std::string &filename,
                                 Size snaplen = 65535) final;

  enum MessageType {
    CHECK_QUEUE,
  };
  class Message : public Module::MessageBase {
  public:
    enum MessageType type;
    ModuleID wireID;
    Message(enum MessageType type, ModuleID wireID)
        : type(type), wireID(wireID) {}
  };

  /**
   * @param bps Set link speed to bps.
   */
  virtual void setLinkSpeed(Size bps) final;

  /**
   * @param max_queue_length Set the maximum queue length.
   * Zero indicates infinite queue.
   */
  virtual void setQueueSize(Size max_queue_length) final;
};

} // namespace E

#endif /* E_LINK_HPP_ */
