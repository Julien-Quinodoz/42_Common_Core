#include "SocketOps.hpp"
#include "Logger.hpp"

/**
 * Creates a TCP socket for server listening
 * 
 * Example: Server startup creates socket fd=5 for port 8080
 * Returns: fd=5 (new socket file descriptor)
 */
int SocketOps::createSocket()
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
	return fd;
}

/**
 * Allows socket to reuse address immediately after restart
 * 
 * Example: Server restart can bind to port 8080 immediately
 * Input: fd=5
 */
void SocketOps::setReuseAddr(int fd)
{
	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("Failed to set SO_REUSEADDR: " + std::string(strerror(errno)));
}

/**
 * Makes socket non-blocking so operations return immediately
 * 
 * Example: Client fd=10 becomes non-blocking
 * recv() returns instantly instead of waiting for data
 */
void SocketOps::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("Failed to set non-blocking: " + std::string(strerror(errno)));
}

/**
 * Binds socket to specific IP address and port
 * 
 * Example: Bind fd=5 to 0.0.0.0:8080
 * Server now "owns" port 8080 and can accept connections
 */
void SocketOps::bindSocket(int fd, const std::string& host, int port)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = host == "0.0.0.0" ? INADDR_ANY : inet_addr(host.c_str());
	
	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
}

/**
 * Marks socket as passive (ready to accept connections)
 * 
 * Example: fd=5 starts listening with queue of 128
 * Browser can now connect to port 8080
 */
void SocketOps::listenSocket(int fd, int backlog)
{
	if (listen(fd, backlog) < 0)
		throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
}

/**
 * Accepts incoming connection and creates new client socket
 * 
 * Example: Browser connects to port 8080
 * Input: server_fd=5
 * Returns: client_fd=10 (NEW socket just for this client)
 * client_addr filled with: {ip: 127.0.0.1, port: 54321}
 */
int SocketOps::acceptConnection(int server_fd, struct sockaddr_in& client_addr)
{
	socklen_t addr_len = sizeof(client_addr);
	return accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
}

/**
 * Closes socket and releases file descriptor
 * 
 * Example: Close client_fd=10 after sending response
 */
void SocketOps::closeSocket(int fd)
{
	close(fd);
}

