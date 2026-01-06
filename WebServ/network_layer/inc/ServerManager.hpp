#pragma once
#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "Webserv.hpp"
#include "ServerConfig.hpp"
#include "ConfigParser.hpp"
#include "Client.hpp"
#include "FdSetManager.hpp"

class ServerManager
{
public:
	ServerManager();
	~ServerManager();
	
	void addServer(const ServerConfig& config);
	void loadConfig(const std::string& config_file);
	void run();
	void stop();
	
private:
	bool _running;
	std::vector<ServerConfig> _servers;
	std::map<int, Client> _clients;
	FdSetManager _fd_manager;
	fd_set _read_set;
	fd_set _write_set;
	
	// Connection statistics
	size_t _total_connections;
	size_t _active_connections;
	
	void initSets();
	void processEvents();
	void handleServerSocket(ServerConfig& server);
	void handleClientRead(int fd);
	void handleClientWrite(int fd);
	void handleCgiRead(int pipe_fd);
	void handleCgiWrite(int pipe_fd);
	void sendCgiBody(int client_fd);
	void readCgiResponse(int client_fd);
	void checkTimeouts();
	void closeClient(int fd);
	void acceptNewConnection(ServerConfig& server);
	ssize_t readFromSocket(int fd, std::string& buffer);
	ssize_t writeToSocket(int fd, const std::string& buffer, size_t& offset);
	
	// Helper to find client by pipe fd
	int findClientByPipe(int pipe_fd, bool is_read_pipe);
};

#endif

