#include "ServerManager.hpp"
#include "SocketOps.hpp"
#include "Logger.hpp"

/**
 * Reads data from socket into buffer (non-blocking)
 * 
 * Example: Reading HTTP request from browser
 * Input: fd=10, buffer=""
 * 1. recv(10, tmp, 40000, 0) → bytes=512
 * 2. tmp = "GET /banana.jpg HTTP/1.1\r\nHost: localhost\r\n\r\n"
 * 3. buffer.append(tmp, 512)
 * 4. buffer = "GET /banana.jpg HTTP/1.1\r\nHost: localhost\r\n\r\n"
 * Returns: 512 (bytes read)
 * 
 * Returns 0 = EOF (client closed)
 * Returns -1 = error or EAGAIN (no data available)
 */
ssize_t ServerManager::readFromSocket(int fd, std::string& buffer)
{
	char tmp[MESSAGE_BUFFER];
	ssize_t bytes = recv(fd, tmp, MESSAGE_BUFFER, 0);
	
	if (bytes > 0)
		buffer.append(tmp, bytes);
	
	return bytes;
}

/**
 * Writes data from buffer to socket (non-blocking, tracks progress)
 * 
 * Example: Sending banana.jpg response
 * Input: fd=10, buffer="HTTP/1.1 200 OK\r\n...[50KB JPEG]", offset=0
 * 
 * Call 1: send(10, buffer+0, 50000) → bytes=8192
 *         offset = 8192 (sent first 8KB)
 * Call 2: send(10, buffer+8192, 41808) → bytes=8192
 *         offset = 16384
 * ...continues until offset >= 50000
 * 
 * Returns bytes sent, or -1 for error/EAGAIN
 */
ssize_t ServerManager::writeToSocket(int fd, const std::string& buffer, size_t& offset)
{
	ssize_t bytes = send(fd, buffer.c_str() + offset, buffer.size() - offset, 0);
	
	if (bytes > 0)
		offset += bytes;
	
	return bytes;
}

/**
 * Closes idle clients that haven't sent/received data in 60 seconds
 * 
 * Example: Client fd=10 connected but inactive
 * - last_activity = 1234567890
 * - now = 1234567950 (60 seconds later)
 * - Timeout! Close fd=10 to free resources
 * 
 * Prevents resource exhaustion from abandoned connections
 */
void ServerManager::checkTimeouts()
{
	time_t now = time(NULL);
	std::vector<int> to_close;
	
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.isTimedOut(now, CONNECTION_TIMEOUT))
		{
			Logger::warn("Client timeout: fd=" + toString(it->first));
			to_close.push_back(it->first);
		}
	}
	
	for (size_t i = 0; i < to_close.size(); ++i)
		closeClient(to_close[i]);
}

/**
 * Cleans up client connection completely
 * 
 * Example: After sending banana.jpg, close fd=10
 * 1. Remove fd=10 from _read_set (stop monitoring reads)
 * 2. Remove fd=10 from _write_set (stop monitoring writes)
 * 3. close(10) - OS releases socket
 * 4. Delete from _clients map (free memory)
 * 5. _active_connections-- (update stats)
 * 
 * fd=10 is now available for next connection
 */
void ServerManager::closeClient(int fd)
{
	_fd_manager.remove(fd, _read_set);
	_fd_manager.remove(fd, _write_set);
	SocketOps::closeSocket(fd);
	_clients.erase(fd);
	
	_active_connections--;
}

