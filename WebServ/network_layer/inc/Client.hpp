#pragma once
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Webserv.hpp"
#include "HttpRequest.hpp"
#include "Response.hpp"
class ServerConfig;

class Client
{
public:
	int socket_fd;
	struct sockaddr_in address;
	time_t last_activity;
	std::string read_buffer;
	std::string write_buffer;
	size_t write_offset;
	size_t parse_offset;  // Track how much has been parsed
	HttpRequest request;
	Response response;
	int listen_fd_owner;
	ServerConfig* server_config;
	
	Client();
	Client(int fd, const struct sockaddr_in& addr);
	
	void updateActivity();
	bool isTimedOut(time_t current_time, int timeout) const;
	void clear();
	std::string getAddressString() const;
	bool requestComplete() const;
};

#endif
