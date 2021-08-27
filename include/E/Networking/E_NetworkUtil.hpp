/*
 * E_NetworkUtil.hpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */

#ifndef E_NETWORKUTIL_HPP_
#define E_NETWORKUTIL_HPP_

#include <E/E_Common.hpp>

namespace E {

class NetworkUtil {
private:
  NetworkUtil();
  virtual ~NetworkUtil();

public:
  /**
   * Calculate checksum once
   * @param buffer Buffer to calculate.
   * @param size Size of buffer.
   * @return Checksum
   */
  static uint16_t one_sum(const uint8_t *buffer, size_t size);

  /**
   * Calculate TCP checksum.
   * @param source Source address (pseudo header)
   * @param dest  Destination address (pseudo header)
   * @param tcp_seg TCP segment
   * @param length TCP length (pseudo header)
   * @return Checksum
   * @note See RFC 793 Checksum
   */
  static uint16_t tcp_sum(uint32_t source, uint32_t dest,
                          const uint8_t *tcp_seg, size_t length);

  /**
   * Converts a uint64_t variable to std::array
   * @param N Size of array
   * @param val uint64_t variable
   * @return Converted array
   */
  template <size_t N>
  static std::array<uint8_t, N> UINT64ToArray(uint64_t val) {
    static_assert(N <= sizeof(uint64_t),
                  "UINT64ToArray requires std::array smaller than uint64_t");

    std::array<uint8_t, N> array;
    for (int k = 0; k < N; k++) {
      array[k] = (val >> (8 * k)) & 0xFF;
    }
    return array;
  }

  /**
   * Converts a std::array to uint64_t.
   * @param N Size of array
   * @param array Array to convert. ipv4_t and mac_t are also okay.
   * @return Converted uint64_t
   */
  template <size_t N>
  static uint64_t arrayToUINT64(const std::array<uint8_t, N> &array) {
    static_assert(N <= sizeof(uint64_t),
                  "arrayToUINT64 requires std::array smaller than uint64_t");

    uint64_t sum = 0;
    for (size_t k = 0; k < N; k++) {
      sum += (((uint64_t)array[k]) << (8 * k));
    }
    return sum;
  }
};

} // namespace E

#endif /* E_NETWORKUTIL_HPP_ */
