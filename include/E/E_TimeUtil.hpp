/**
 * @file   E_TimeUtil.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::TimeUtil
 */


#ifndef E_TIMEUTIL_HPP_
#define E_TIMEUTIL_HPP_

#include <E/E_Common.hpp>

namespace E
{
/**
 * @brief TimerUtil provides utilities for time calculation.
 * Using this class instead of Time variable directly provides
 * better compatibility for further improvement.
 *
 * @note Current unit of Time is unsigned 64bit integer in nanoseconds.
 * However, it may be extended to picosecond resolution or real number resolution.
 * TimerUtil provides consistent view of Time.
 */
class TimeUtil
{
private:
	TimeUtil();
	virtual ~TimeUtil();
public:
	/**
	 * @brief Enumerations for time unit.
	 *
	 */
	enum TimeUnit
	{
		NSEC,
		USEC,
		MSEC,
		SEC,
		MINUTE,
		HOUR,
		DAY,
		NONE,
	};
	/**
	 * @brief Converts time unit string to TimeUnit enumeration.
	 *
	 * @param unit Time unit string (e.g. "NSEC", "USEC", etc.)
	 *
	 * @return TimeUtil::TimeUnit enumeration for given time unit string
	 *
	 */
	static enum TimeUnit stringToTimeUnit(const std::string& unit);
	/**
	 * @brief Converts TimeUnit enumeration to time unit string.
	 *
	 * @param unit TimeUtil::TimeUnit enumeration
	 *
	 * @return Time unit string for given TimeUnit enumeration
	 *
	 */
	static std::string timeUnitToString(enum TimeUnit unit);

	/**
	 * @brief Calculates multiplier for given time unit from the minimal TimeUnit.
	 * Current minimal TimeUnit is NSEC.
	 * For example, multiplier for millisecond is 1,000,000 and for microsecond is 1,000.
	 *
	 * @param unit TimeUtil::TimeUnit enumeration
	 *
	 * @return Multiplier for given TimeUtil::TimeUnit enumeration
	 *
	 */
	static Size getMultiplier(enum TimeUnit unit);
	/**
	 * @brief Produces nanosecond time from a time in given time unit.
	 *
	 * @param time Time value in given time unit
	 * @param unit TimeUtil::TimeUnit enumeration
	 *
	 * @return Time value (currently, in nanosecond)
	 *
	 */
	static Time makeTime(Size time, enum TimeUnit unit);
	/**
	 * @brief Converts nanosecond time value to a time in given time unit.
	 *
	 * @param time Time value in nanosecond
	 * @param unit TimeUtil::TimeUnit enumeration
	 *
	 * @return Time value in given time unit
	 *
	 */
	static Size getTime(Time time, enum TimeUnit unit);
	/**
	 * @brief Converts nanosecond time value to a time in given time unit in string.
	 *
	 * @param time Time value in nanosecond
	 * @param unit TimeUtil::TimeUnit enumeration
	 *
	 * @return Time string in given time unit
	 *
	 */
	static std::string printTime(Time time, enum TimeUnit unit);
	//static Time parseTime(const std::string& time);
	//static std::string timeToString(Time time);
	//static std::string timeToString(Time time, enum TimeUnit unit);
};

}


#endif /* E_TIMEUTIL_HPP_ */
