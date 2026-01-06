#include "ServerManager.hpp"
#include "SocketOps.hpp"
#include "Logger.hpp"
#include <map>
#include <utility>

/**
 * Initializes ServerManager with empty fd_sets
 * 
 * Example: Creates manager with:
 * - _read_set = {} (empty)
 * - _write_set = {} (empty)
 * - _clients = {} (no clients yet)
 */
ServerManager::ServerManager() : _running(false), _total_connections(0), _active_connections(0)
{
	_fd_manager.clear(_read_set);
	_fd_manager.clear(_write_set);
}

ServerManager::~ServerManager()
{
	stop();
}

void ServerManager::addServer(const ServerConfig& config)
{
	_servers.push_back(config);
}

/**
 * Loads config file and creates listening sockets for each server
 * 
 * Example: config/default.conf has 2 servers
 * Result:
 * - Server 0: fd=5 listening on 0.0.0.0:8080
 * - Server 1: fd=6 listening on 0.0.0.0:8081
 * Both sockets are now ready to accept() connections
 */
void ServerManager::loadConfig(const std::string& config_file)
{
	ConfigParser parser;
	
	try
	{
		parser.createCluster(config_file);
		_servers = parser.getServers();
		
		Logger::info("Loaded " + toString(_servers.size()) + " server(s) from config");
		
		// Group servers by (host, port) for virtual hosts
		std::map<std::pair<in_addr_t, uint16_t>, int> socket_map;
		
		for (size_t i = 0; i < _servers.size(); ++i)
		{
			std::pair<in_addr_t, uint16_t> key(_servers[i].getHost(), _servers[i].getPort());
			
			// If socket already exists for this (host, port), reuse it
			if (socket_map.find(key) != socket_map.end())
			{
				_servers[i].setFd(socket_map[key]);
				Logger::info("Server " + toString(i) + ": " + _servers[i].getServerName() + 
					" sharing socket on " + toString(_servers[i].getPort()));
			}
			else
			{
				// Create new socket for this (host, port) combination
				_servers[i].setupServer();
				socket_map[key] = _servers[i].getFd();
				Logger::info("Server " + toString(i) + ": " + _servers[i].getServerName() + 
					" listening on " + toString(_servers[i].getPort()));
			}
		}
	}
	catch (std::exception& e)
	{
		Logger::error("Config parsing failed: " + std::string(e.what()));
		throw;
	}
}

/**
 * Adds all server listening sockets to read_set for select()
 * 
 * Example: After loadConfig()
 * Before: _read_set = {}
 * After:  _read_set = {5, 6}  (monitoring both server sockets)
 * _max_fd = 6
 */
void ServerManager::initSets()
{
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		int fd = _servers[i].getFd();
		_fd_manager.add(fd, _read_set);
		_fd_manager.updateMaxFd(fd);
	}
}

void ServerManager::run()
{
	Logger::info("Starting ServerManager...");
	initSets();
	_running = true;
	
	while (_running)
	{
		processEvents();
		checkTimeouts();
	}
}

void ServerManager::stop()
{
	_running = false;
	
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		SocketOps::closeSocket(it->first);
	_clients.clear();
	
	for (size_t i = 0; i < _servers.size(); ++i)
		SocketOps::closeSocket(_servers[i].getFd());
	
	Logger::info("ServerManager stopped");
}

/**
 * Main event loop: calls select() and dispatches events
 * 
 * Example flow for "GET /banana.jpg":
 * 1. select() waits for activity on {5, 6, 10}
 * 2. Browser sends request → fd=10 becomes readable
 * 3. read_cpy = {10}, write_cpy = {}
 * 4. Loop finds fd=10 in read_cpy
 * 5. fd=10 is in _clients → calls handleClientRead(10)
 * 6. After parsing, fd=10 added to write_set
 * 7. Next select(): write_cpy = {10}
 * 8. Loop finds fd=10 in write_cpy → calls handleClientWrite(10)
 */
void ServerManager::processEvents()
{
	fd_set read_cpy = _read_set;
	fd_set write_cpy = _write_set;
	
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	int ready = select(_fd_manager.getMaxFd() + 1, &read_cpy, &write_cpy, NULL, &timeout);
	
	if (ready < 0)
	{
		_running = false;
		return;
	}
	
	if (ready == 0)
		return;
	
	for (int fd = 0; fd <= _fd_manager.getMaxFd(); ++fd)
	{
		if (_fd_manager.isSet(fd, read_cpy))
		{
			for (size_t i = 0; i < _servers.size(); ++i)
			{
				if (_servers[i].getFd() == fd)
				{
					handleServerSocket(_servers[i]);
					break;
				}
			}
			
			if (_clients.find(fd) != _clients.end())
				handleClientRead(fd);
			else
				handleCgiRead(fd);
		}
		
		if (_fd_manager.isSet(fd, write_cpy))
		{
			if (_clients.find(fd) != _clients.end())
				handleClientWrite(fd);
			else
				handleCgiWrite(fd);
		}
	}
}

