#pragma once
#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <cstdarg>
#include <cstring>
#include <cerrno>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define CONNECTION_TIMEOUT 60
#define MESSAGE_BUFFER 40000
#define MAX_CONNECTIONS 1024
#define MAX_URI_LENGTH 4096
#define MAX_CONTENT_LENGTH 30000000

std::string statusCodeString(short);
std::string getErrorPage(short);
int buildHtmlIndex(std::string &, std::vector<uint8_t> &, size_t &);
int ft_stoi(std::string str);
unsigned int fromHexToDec(const std::string& nb);

template <typename T>
std::string toString(const T val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}

#endif

