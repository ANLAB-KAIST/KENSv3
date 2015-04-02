/**
 * @file   E_Log.hpp
 * @Author leeopop (dlrmsghd@gmail.com)
 * @date   November, 2014
 * @brief  Header for E::Log
 */

#ifndef E_LOG_HPP_
#define E_LOG_HPP_

#include <E/E_Common.hpp>

namespace E {
/**
 * @brief Log provides log-related utilities.
 * We recommend you to inherit this class privately,
 * and it will enable Log::print_log in your namespace.
 *
 * @note You cannot use both Log and NetworkLog simultaneously.
 * @see E::NetworkLog
 */
class Log {
private:
	int level;
	static const char* LEVEL_STR[];

public:
	/**
	 * @brief Constructs a Log instance.
	 *
	 */
	Log();

	/**
	 * @brief Constructs a Log instance with log level.
	 *
	 * @param level Log level
	 */
	Log(int level);

	/**
	 * @brief Destructs a Log instance.
	 *
	 */
	~Log();

	/**
	 * @brief Enumerations for log levels.
	 *
	 */
	enum LOG_LEVEL {
		ERR, WARN, LOG, INFO, DEBUG, LEVEL_COUNT,
	};

protected:
	/**
	 * @brief Prints log with specified log level and format.
	 * Log::print_log prints all logs that has lower log level than the specified log level.
	 * For example, when log level is set to LOG, it prints ERR, WARN, and LOG logs.
	 *
	 * @note NetworkLog::print_log
	 *
	 * @param level Log level
	 * @param format Format string
	 * @param ... Print arguments for format string
	 *
	 */
	void print_log(int level, const char* format, ...) __attribute__((format(printf,3,4)));

public:
	/**
	 * @brief Default log level.
	 *
	 */
	static int defaultLevel;
};

}

#endif /* E_LOG_HPP_ */
