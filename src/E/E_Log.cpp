/*
 * E_Log.cpp
 *
 *  Created on: 2014. 11. 2.
 *      Author: Keunhong Lee
 */

#include <E/E_Log.hpp>

namespace E
{

const char* Log::LEVEL_STR[LEVEL_COUNT] =
{
		"ERR",
		"WARN",
		"LOG",
		"INFO",
		"DEBUG",
};

int Log::defaultLevel = Log::LOG_LEVEL;

Log::Log()
{
	this->level = defaultLevel;
}

Log::Log(int level)
{
	this->level = level;
}
Log::~Log()
{

}

void Log::print_log(int level, const char* format, ...)
{
	if(level > this->level)
		return;
	printf("[%s] ", this->LEVEL_STR[level]);
	va_list args;
	va_start (args, format);
	vprintf (format, args);
	va_end (args);
	printf("\n");
	fflush(stdout);
}

}


