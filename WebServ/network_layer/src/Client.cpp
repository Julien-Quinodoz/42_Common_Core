#include "Client.hpp"

/**
 * Default constructor (unused, for map compatibility)
 */
Client::Client() : socket_fd(-1), last_activity(0), write_offset(0), parse_offset(0)
{
	memset(&address, 0, sizeof(address));
	listen_fd_owner = -1;
	server_config = NULL;
}

/**
 * Creates Client object for new connection
 * 
 * Example: Browser connects to request /banana.jpg
 * Input: fd=10, addr={ip:127.0.0.1, port:54321}
 * Creates:
 * - socket_fd = 10
 * - read_buffer = "" (empty, will fill when data arrives)
 * - write_buffer = "" (empty, will fill with response)
 * - write_offset = 0 (no data sent yet)
 * - last_activity = current timestamp
 */
Client::Client(int fd, const struct sockaddr_in& addr) 
	: socket_fd(fd), address(addr), last_activity(time(NULL)), write_offset(0), parse_offset(0)
{
	listen_fd_owner = -1;
	server_config = NULL;
}

/**
 * Updates last activity timestamp (called on every read/write)
 * 
 * Example: Browser sends data or we send response
 * last_activity = 1234567890 → 1234567920 (updated to now)
 * Prevents timeout for active connections
 */
void Client::updateActivity()
{
	last_activity = time(NULL);
}

/**
 * Checks if client has been idle too long
 * 
 * Example: Check if fd=10 timed out
 * last_activity = 1234567890
 * current_time = 1234567950
 * timeout = 60
 * Returns: (1234567950 - 1234567890) > 60 = false (still ok)
 * 
 * If 61+ seconds pass → returns true → close connection
 */
bool Client::isTimedOut(time_t current_time, int timeout) const
{
	return (current_time - last_activity) > timeout;
}

/**
 * Resets client for connection reuse (keep-alive)
 * 
 * Example: After sending banana.jpg, prepare for next request
 * Before: read_buffer="GET /banana.jpg...", write_buffer="HTTP/1.1 200..."
 * After:  read_buffer="", write_buffer="", write_offset=0
 * 
 * Also shrinks buffers if they grew too large (memory optimization)
 */
void Client::clear()
{
	// Optimize memory usage by shrinking buffers if they're too large (C++98 compatible)
	if (read_buffer.capacity() > MESSAGE_BUFFER * 2)
	{
		std::string tmp = read_buffer;
		read_buffer.swap(tmp);
	}
	if (write_buffer.capacity() > MESSAGE_BUFFER * 2)
	{
		std::string tmp = write_buffer;
		write_buffer.swap(tmp);
	}
	
	read_buffer.clear();
	write_buffer.clear();
	write_offset = 0;
	parse_offset = 0;
	request.clear();
	response.clear();
	listen_fd_owner = -1;
	server_config = NULL;
}

/**
 * Checks if HTTP request parsing is complete
 * 
 * Example: Reading "GET /banana.jpg HTTP/1.1\r\n\r\n"
 * After receiving \r\n\r\n → returns true
 * Ready to build response
 */
bool Client::requestComplete() const
{
	return const_cast<HttpRequest&>(request).parsingCompleted();
}

/**
 * Formats client address for logging
 * 
 * Example: address={ip:127.0.0.1, port:54321}
 * Returns: "127.0.0.1:54321"
 */
std::string Client::getAddressString() const
{
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &address.sin_addr, ip, INET_ADDRSTRLEN);
	return std::string(ip) + ":" + toString(ntohs(address.sin_port));
}

