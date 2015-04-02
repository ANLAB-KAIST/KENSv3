/**
 * @file   E_Common.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  This header contains standard C++11 headers and compatibility definitions.
 */

#ifndef E_COMMON_HPP_
#define E_COMMON_HPP_

#include <iostream>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <queue>
#include <stack>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cmath>
#include <random>
#include <memory>
#include <cstring>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <regex>

namespace E
{
typedef uint64_t Time;
typedef uint64_t UUID;
typedef uint64_t Priority;
typedef size_t CPUID;
typedef size_t Size;
typedef double Real;
}

namespace std
{
template <class K, class V>
struct hash<std::pair<K,V>>
{
	size_t operator()(const std::pair<K,V>& ns) const
	{
		return hash<K>()(ns.first)* hash<V>()(ns.second)
				+ hash<K>()(ns.first) + hash<V>()(ns.second);
	}
};
}

#ifndef LOG_LEVEL
#define LOG_LEVEL WARN
#endif

#endif /* E_COMMON_HPP_ */
