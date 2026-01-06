#pragma once
#ifndef SOCKETOPS_HPP
#define SOCKETOPS_HPP

#include "Webserv.hpp"

namespace SocketOps
{
	int createSocket();
	void setReuseAddr(int fd);
	void setNonBlocking(int fd);
	void bindSocket(int fd, const std::string& host, int port);
	void listenSocket(int fd, int backlog = 128);
	int acceptConnection(int server_fd, struct sockaddr_in& client_addr);
	void closeSocket(int fd);
}

#endif

