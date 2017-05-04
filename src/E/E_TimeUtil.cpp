/*
 * E_TimerModule.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: Keunhong Lee
 */


#include <E/E_Common.hpp>
#include <E/E_TimeUtil.hpp>

namespace E
{

TimeUtil::TimeUtil()
{

}
TimeUtil::~TimeUtil()
{

}

enum TimeUtil::TimeUnit TimeUtil::stringToTimeUnit(const std::string& unit)
{
	if(unit == "nsec" || unit == "NSEC")
	{
		return TimeUnit::NSEC;
	}
	else if(unit == "usec" || unit == "USEC")
	{
		return TimeUnit::USEC;
	}
	else if(unit == "msec" || unit == "MSEC")
	{
		return TimeUnit::MSEC;
	}
	else if(unit == "sec" || unit == "SEC")
	{
		return TimeUnit::SEC;
	}
	else if(unit == "minute" || unit == "MINUTE")
	{
		return TimeUnit::MINUTE;
	}
	else if(unit == "hour" || unit == "HOUR")
	{
		return TimeUnit::HOUR;
	}
	else if(unit == "day" || unit == "DAY")
	{
		return TimeUnit::DAY;
	}
	assert(0);
	return NONE;
}
std::string TimeUtil::timeUnitToString(enum TimeUnit unit)
{
	switch(unit)
	{
	case NSEC:
		return "NSEC";
	case USEC:
		return "USEC";
	case MSEC:
		return "MSEC";
	case SEC:
		return "SEC";
	case MINUTE:
		return "MINUTE";
	case HOUR:
		return "HOUR";
	case DAY:
		return "DAY";
	default:
		assert(0);
		return std::string();
	}
}

Size TimeUtil::getMultiplier(enum TimeUnit unit)
{
	Size multiplier = 1;

	switch(unit)
	{
	case DAY:
		multiplier *= 24;
	case HOUR:
		multiplier *= 60;
	case MINUTE:
		multiplier *= 60;
	case SEC:
		multiplier *= 1000;
	case MSEC:
		multiplier *= 1000;
	case USEC:
		multiplier *= 1000;
	case NSEC:
	default:
		break;
	}

	return multiplier;
}

Time TimeUtil::makeTime(Size time, enum TimeUnit unit)
{
	Size multiplier = getMultiplier(unit);

	return time * multiplier;
}

Size TimeUtil::getTime(Time time, enum TimeUnit unit)
{
	Size multiplier = getMultiplier(unit);
	return time / multiplier;
}

std::string TimeUtil::printTime(Time time, enum TimeUnit unit)
{
	Size multiplier = 1;

	switch(unit)
	{
	case DAY:
		multiplier *= 24;
	case HOUR:
		multiplier *= 60;
	case MINUTE:
		multiplier *= 60;
	case SEC:
		multiplier *= 1000;
	case MSEC:
		multiplier *= 1000;
	case USEC:
		multiplier *= 1000;
	case NSEC:
	default:
		break;
	}

	char buf[128];
	snprintf(buf, sizeof(buf), "%lu", time/multiplier);
	return std::string(buf);
}

}

