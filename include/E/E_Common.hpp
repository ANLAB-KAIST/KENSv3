/**
 * @file   E_Common.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  This header contains standard C++11 headers and compatibility
 * definitions.
 */

#ifndef E_COMMON_HPP_
#define E_COMMON_HPP_

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <condition_variable>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <stack>
#include <thread>
#include <unordered_map>
#include <unordered_set>

namespace E {
typedef uint64_t Time;
typedef uint64_t UUID;
typedef uint64_t Priority;
typedef size_t CPUID;
typedef size_t Size;
typedef double Real;
} // namespace E

namespace std {
template <class K, class V> struct hash<std::pair<K, V>> {
  size_t operator()(const std::pair<K, V> &ns) const {
    return hash<K>()(ns.first) * hash<V>()(ns.second) + hash<K>()(ns.first) +
           hash<V>()(ns.second);
  }
};
} // namespace std

#ifndef LOG_LEVEL
#define LOG_LEVEL WARN
#endif

#endif /* E_COMMON_HPP_ */
