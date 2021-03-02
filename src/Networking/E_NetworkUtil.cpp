/*
 * E_NetworkUtil.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#include <E/Networking/E_NetworkUtil.hpp>
#include <arpa/inet.h>

namespace E {

NetworkUtil::NetworkUtil() {}
NetworkUtil::~NetworkUtil() {}

uint16_t NetworkUtil::one_sum(const uint8_t *buffer, size_t size) {
  bool upper = true;
  uint32_t sum = 0;
  for (size_t k = 0; k < size; k++) {
    if (upper) {
      sum += buffer[k] << 8;
    } else {
      sum += buffer[k];
    }

    upper = !upper;

    sum = (sum & 0xFFFF) + (sum >> 16);
  }
  sum = (sum & 0xFFFF) + (sum >> 16);
  return (uint16_t)sum;
}
#ifdef HAVE_PRAGMA_PACK
#pragma pack(push, 1)
#endif
struct pseudoheader {
  uint32_t source;
  uint32_t destination;
  uint8_t zero;
  uint8_t protocol;
  uint16_t length;
}
#if defined(HAVE_ATTR_PACK)
__attribute__((packed));
#elif defined(HAVE_PRAGMA_PACK)
;
#pragma pack(pop)
#else
#error "Compiler must support packing"
#endif
uint16_t NetworkUtil::tcp_sum(uint32_t source, uint32_t dest,
                              const uint8_t *tcp_seg, size_t length) {
  if (length < 20)
    return 0;
  struct pseudoheader pheader;
  pheader.source = source;
  pheader.destination = dest;
  pheader.zero = 0;
  pheader.protocol = IPPROTO_TCP;
  pheader.length = htons(length);

  uint32_t sum = one_sum((uint8_t *)&pheader, sizeof(pheader));
  sum += one_sum(tcp_seg, length);
  sum = (sum & 0xFFFF) + (sum >> 16);
  return (uint16_t)sum;
}

} // namespace E
