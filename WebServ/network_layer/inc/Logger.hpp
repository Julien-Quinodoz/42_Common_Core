#pragma once
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "Webserv.hpp"

#define RESET "\x1B[0m"
#define RED "\x1B[31m"
#define YELLOW "\x1B[33m"
#define CYAN "\x1B[36m"
#define LIGHT_BLUE "\x1B[94m"
#define DARK_GREY "\x1B[90m"

enum LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger
{
public:
	static void log(LogLevel level, const std::string& msg);
	static void debug(const std::string& msg);
	static void info(const std::string& msg);
	static void warn(const std::string& msg);
	static void error(const std::string& msg);
	static void performance(const std::string& operation, double time_ms);
	
private:
	static std::string getTimestamp();
	static std::string getLevelColor(LogLevel level);
	static std::string getLevelName(LogLevel level);
};

#endif

