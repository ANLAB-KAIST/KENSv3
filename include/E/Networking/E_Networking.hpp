/**
 * @file   E_Networking.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::NetworkModule and E::NetworkSystem
 */

#ifndef E_NETWORKING_HPP_
#define E_NETWORKING_HPP_

#include <E/E_Common.hpp>
#include <E/E_Module.hpp>
#include <E/E_System.hpp>
#include <E/Networking/E_NetworkLog.hpp>

namespace E {
class Packet;
class NetworkSystem;

/**
 * @brief NetworkModule can allocate/deallocate Packets.
 */
class NetworkModule {
private:
  NetworkSystem *system;
  std::string name;

public:
  /**
   * @brief NetworkModule is registered to the NetworkSystem
   * when it is created.
   * @param name Name of this NetworkModule.
   * @param system NetworkSystem to be registered.
   */
  NetworkModule(std::string name, NetworkSystem *system);
  virtual ~NetworkModule();

  /**
   * @return Name of this module.
   * @note You cannot override this function.
   */
  virtual std::string getModuleName() final;

  /**
   * @return NetworkSystem this module is registered to.
   * @note You cannot override this function.
   */
  virtual NetworkSystem *getNetworkSystem() final;

protected:
  /**
   * @brief Make a clone of the given Packet.
   * @param packet Packet to be cloned.
   * @return Cloned Packet
   * @note You cannot override this function.
   */
  // virtual Packet *clonePacket(Packet *packet) final;

  friend class NetworkSystem;
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

  std::unordered_map<std::string, NetworkModule *> moduleNameMap;

public:
  NetworkSystem();
  virtual ~NetworkSystem();
};

} // namespace E

#endif /* E_NETWORKING_HPP_ */
