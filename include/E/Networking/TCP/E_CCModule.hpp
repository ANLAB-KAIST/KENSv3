#ifndef E_CCModule_HPP_
#define E_CCModule_HPP_

#include <E/Networking/E_Packet.hpp>

namespace E {

constexpr int TCP_MSS = 512;


class CCModule {
public:
  virtual int packetSent(Packet &&packet) { return TCP_MSS; }
  virtual int packetArrived(Packet &&packet) { return TCP_MSS; }
  virtual int packetTimeout(Packet &&packet) { return TCP_MSS; }
};

} // namespace E
#endif