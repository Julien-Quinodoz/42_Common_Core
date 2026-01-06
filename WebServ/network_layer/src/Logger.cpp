#include "Logger.hpp"
#include <iomanip>
#include <sstream>

std::string Logger::getTimestamp()
{
	time_t now = time(NULL);
	char buf[20];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
	return std::string(buf);
}

std::string Logger::getLevelColor(LogLevel level)
{
	switch (level)
	{
		case DEBUG: return DARK_GREY;
		case INFO: return CYAN;
		case WARN: return YELLOW;
		case ERROR: return RED;
		default: return RESET;
	}
}

std::string Logger::getLevelName(LogLevel level)
{
	switch (level)
	{
		case DEBUG: return "DEBUG";
		case INFO: return "INFO";
		case WARN: return "WARN";
		case ERROR: return "ERROR";
		default: return "UNKNOWN";
	}
}

void Logger::log(LogLevel level, const std::string& msg)
{
	std::cout << getLevelColor(level) 
			  << "[" << getTimestamp() << "] "
			  << "[" << getLevelName(level) << "] "
			  << msg << RESET << std::endl;
}

void Logger::debug(const std::string& msg) { log(DEBUG, msg); }
void Logger::info(const std::string& msg) { log(INFO, msg); }
void Logger::warn(const std::string& msg) { log(WARN, msg); }
void Logger::error(const std::string& msg) { log(ERROR, msg); }

// Performance monitoring
void Logger::performance(const std::string& operation, double time_ms)
{
	std::stringstream ss;
	ss << operation << " took " << std::fixed << std::setprecision(2) << time_ms << "ms";
	log(INFO, ss.str());
}

