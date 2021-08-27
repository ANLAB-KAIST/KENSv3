/**
 * @file   E_Networking.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for  E::NetworkSystem
 */

#ifndef E_NETWORKING_HPP_
#define E_NETWORKING_HPP_

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/E_System.hpp>
#include <E/Networking/E_NetworkLog.hpp>
#include <E/Networking/E_Wire.hpp>

namespace E {
class Packet;

/**
 * @brief NetworkModule is a abstract class for wired connection.
 */
class NetworkModule : public Module {
public:
  NetworkModule(System &system);
  int connectWire(const ModuleID moduleID);
  size_t getPortCount() { return ports.size(); }

protected:
  std::vector<ModuleID> ports;
};

/**
 * @brief NetworkSystem is a kind of System
 * which has extensions for handling Packet network.
 * @see System.
 */
class NetworkSystem : public System, private NetworkLog {
private:
  UUID packetUUIDStart;
  std::unordered_set<UUID> packetUUIDSet;
  void freePacketUUID(UUID uuid);
  UUID allocatePacketUUID();

public:
  NetworkSystem();
  virtual ~NetworkSystem();
  std::pair<std::shared_ptr<Wire>, std::pair<int, int>>
  addWire(NetworkModule &left, NetworkModule &right,
          Time propagationDelay = 1000000, Size bps = 1000000000UL,
          bool limit_speed = true);

  Size getWireSpeed(const ModuleID moduleID);
};

} // namespace E

#endif /* E_NETWORKING_HPP_ */
