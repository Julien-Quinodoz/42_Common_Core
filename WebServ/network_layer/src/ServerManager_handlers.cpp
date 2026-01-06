#include "ServerManager.hpp"
#include "SocketOps.hpp"
#include "Logger.hpp"
#include <sys/wait.h>
#include <cstring>

void ServerManager::handleServerSocket(ServerConfig& server)
{
	acceptNewConnection(server);
}

/**
 * Accepts new client connection and adds to monitoring
 * 
 * Example: Browser connects to request /banana.jpg
 * Input: server_fd=5 (listening socket)
 * 1. accept() creates client_fd=10
 * 2. Set fd=10 to non-blocking
 * 3. Create Client object with empty buffers
 * 4. Add fd=10 to _read_set (monitor for incoming data)
 * 5. _clients[10] = {socket_fd:10, read_buffer:"", write_buffer:""}
 * 
 * Now select() will notify us when browser sends HTTP request
 */
void ServerManager::acceptNewConnection(ServerConfig& server)
{
	struct sockaddr_in client_addr;
	int client_fd = SocketOps::acceptConnection(server.getFd(), client_addr);

	if (client_fd < 0)
		return;

	SocketOps::setNonBlocking(client_fd);

	Client client(client_fd, client_addr);
	client.listen_fd_owner = server.getFd();
	client.server_config = &server;
	_clients[client_fd] = client;
	_fd_manager.add(client_fd, _read_set);
	_fd_manager.updateMaxFd(client_fd);

	_total_connections++;
	_active_connections++;

	Logger::info("New connection: fd=" + toString(client_fd) + " from " + client.getAddressString() +
		" (Total: " + toString(_total_connections) + ", Active: " + toString(_active_connections) + ")");
}

/**
 * Reads HTTP request from client socket
 * 
 * Example: Browser sends "GET /banana.jpg HTTP/1.1\r\n..."
 * Input: fd=10 (readable according to select)
 * 1. recv() reads data into client.read_buffer
 * 2. read_buffer = "GET /banana.jpg HTTP/1.1\r\nHost: localhost\r\n\r\n"
 * 3. Feed to HTTP parser
 * 4. Parser extracts: method=GET, path=/banana.jpg
 * 5. Build response (read banana.jpg file)
 * 6. Add fd=10 to write_set (ready to send response)
 * 
 * Next select() will notify when fd=10 is writable
 */
void ServerManager::handleClientRead(int fd)
{
	Client& client = _clients[fd];
	std::string& buffer = client.read_buffer;

	ssize_t bytes = readFromSocket(fd, buffer);

	if (bytes < 0)
	{
		Logger::error("Read error on fd=" + toString(fd));
		closeClient(fd);
		return;
	}

	if (bytes == 0)
	{
		Logger::info("Connection closed by client: fd=" + toString(fd));
		closeClient(fd);
		return;
	}

	client.updateActivity();

	// Feed ONLY new data to HTTP parser (not the entire buffer)
	size_t new_data_size = buffer.size() - client.parse_offset;
	if (new_data_size > 0)
	{
		client.request.feed((char*)buffer.c_str() + client.parse_offset, new_data_size);
		client.parse_offset = buffer.size();
	}

	// If request is complete, build response
	if (client.requestComplete())
	{
		// Select server based on listening socket and Host header
		ServerConfig* server_config = client.server_config;
		if (server_config)
		{
			// If multiple servers share same (host,port), try match by server_name from Host header
			std::string host_name = client.request.getServerName();
			if (!host_name.empty())
			{
				in_addr_t host = server_config->getHost();
				uint16_t port = server_config->getPort();
				for (size_t i = 0; i < _servers.size(); ++i)
				{
					if (_servers[i].getHost() == host && _servers[i].getPort() == port)
					{
						if (_servers[i].getServerName() == host_name)
						{
							server_config = &_servers[i];
							break;
						}
					}
				}
			}
			client.response.setRequest(client.request);
			client.response.setServer(*server_config);
			client.response.buildResponse();

			// If CGI is active, add pipes to select sets
			if (client.response.getCgiState() == 1)
			{
				// Add pipe_in[1] to write_set (to send POST body to CGI)
				// Add pipe_out[0] to read_set (to read CGI response)
				_fd_manager.add(client.response.cgi_obj.pipe_in[1], _write_set);
				_fd_manager.add(client.response.cgi_obj.pipe_out[0], _read_set);
				Logger::info("CGI detected, pipes added to select sets for fd=" + toString(fd) +
					" (pipe_out[0]=" + toString(client.response.cgi_obj.pipe_out[0]) +
					", pipe_in[1]=" + toString(client.response.cgi_obj.pipe_in[1]) + ")");
			}
			else
			{
				client.write_buffer = client.response.getRes();
				_fd_manager.add(fd, _write_set);
				Logger::info("Request parsed, response ready for fd=" + toString(fd));
			}
		}
	}
}

/**
 * Sends HTTP response to client socket
 * 
 * Example: Sending banana.jpg to browser
 * Input: fd=10 (writable according to select)
 * 1. write_buffer = "HTTP/1.1 200 OK\r\n...Content-Length: 50000\r\n\r\n[50KB of JPEG data]"
 * 2. send() writes data, may be partial: bytes=8192
 * 3. write_offset = 8192 (track progress)
 * 4. Still more data? Keep fd=10 in write_set
 * 5. Next select() → write again from offset 8192
 * 6. When write_offset >= 50000 → complete!
 * 7. Remove from write_set, close connection
 * 
 * Browser now displays banana.jpg
 */
void ServerManager::handleClientWrite(int fd)
{
	Client& client = _clients[fd];

	ssize_t bytes = writeToSocket(fd, client.write_buffer, client.write_offset);

	if (bytes < 0)
	{
		Logger::error("Write error on fd=" + toString(fd));
		closeClient(fd);
		return;
	}

	client.updateActivity();

	if (client.write_offset >= client.write_buffer.size())
	{
		_fd_manager.remove(fd, _write_set);

		// If CGI is still active (state == 1), keep connection open
		// select() will notify us when more data is available on pipe
		if (client.response.getCgiState() == 1)
			return;

			// CGI is done (state == 2), safe to close or keep-alive
			// For CGI responses, it's safer to close the connection to ensure browsers receive all data
			// Some browsers may buffer responses and wait for connection close
		closeClient(fd);
	}
}

/**
 * Finds which client owns a CGI pipe
 * 
 * Example: CGI script for client fd=10 has pipes
 * - pipe_out[0] = 15 (read CGI output)
 * - pipe_in[1] = 16 (write POST data to CGI)
 * 
 * When select() says fd=15 is readable:
 * findClientByPipe(15, true) → returns 10
 * We know client 10's CGI has output ready
 */
int ServerManager::findClientByPipe(int pipe_fd, bool is_read_pipe)
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.response.getCgiState() == 1)
		{
			if (is_read_pipe && it->second.response.cgi_obj.pipe_out[0] == pipe_fd)
				return it->first;
			if (!is_read_pipe && it->second.response.cgi_obj.pipe_in[1] == pipe_fd)
				return it->first;
		}
	}
	return -1;
}

void ServerManager::handleCgiRead(int pipe_fd)
{
	int client_fd = findClientByPipe(pipe_fd, true);
	if (client_fd >= 0)
		readCgiResponse(client_fd);
}

void ServerManager::handleCgiWrite(int pipe_fd)
{
	int client_fd = findClientByPipe(pipe_fd, false);
	if (client_fd >= 0)
		sendCgiBody(client_fd);
}

/**
 * Sends POST body to CGI script via pipe
 * 
 * Example: POST request uploading banana image
 * Client fd=10, pipe_in[1]=16
 * 1. req_body = "[50KB of image data]"
 * 2. write(16, data, 50000) → bytes_sent=8192 (partial)
 * 3. req_body = req_body.substr(8192) (remaining 41808 bytes)
 * 4. Keep pipe_in[1]=16 in write_set
 * 5. Next select() → write more data
 * 6. When all sent → close(16), remove from write_set
 * 
 * CGI script now has full POST body via stdin
 */
void ServerManager::sendCgiBody(int client_fd)
{
	Client& client = _clients[client_fd];
	CgiHandler& cgi = client.response.cgi_obj;
	std::string& req_body = client.request.getBody();

	if (req_body.length() == 0)
	{
		// No body to send, close pipe and remove from write set
		_fd_manager.remove(cgi.pipe_in[1], _write_set);
		close(cgi.pipe_in[1]);
		return;
	}

	ssize_t bytes_sent = write(cgi.pipe_in[1], req_body.c_str(), req_body.length());

	if (bytes_sent < 0)
	{
		client.updateActivity();
		return;
	}
	else if ((size_t)bytes_sent == req_body.length())
	{
		// All body sent, close pipe and remove from write set
		_fd_manager.remove(cgi.pipe_in[1], _write_set);
		close(cgi.pipe_in[1]);
		req_body.clear();
	}
	else
	{
		// Partial send, update body
		req_body = req_body.substr(bytes_sent);
		client.updateActivity();
	}
}

/**
 * Reads CGI script output via pipe and sends to client
 * 
 * Example: CGI generates dynamic HTML for /banana.php
 * Client fd=10, pipe_out[0]=15
 * 1. CGI writes "HTTP/1.1 200 OK\r\n...HTML content..."
 * 2. select() says fd=15 readable
 * 3. read(15, buffer, 80000) → bytes_read=4096
 * 4. Append to response_content
 * 5. Check if headers complete (\r\n\r\n found)
 * 6. Start sending to client (add fd=10 to write_set)
 * 7. Continue reading until EOF
 * 8. When EOF → close(15), wait for CGI process
 * 9. Final response sent via handleClientWrite()
 */
void ServerManager::readCgiResponse(int client_fd)
{
	Client& client = _clients[client_fd];
	CgiHandler& cgi = client.response.cgi_obj;

	char buffer[MESSAGE_BUFFER * 2];
	ssize_t bytes_read;

	// Try to read as much data as possible in one go (non-blocking)
	// Keep reading until we get EAGAIN or EOF
	while (true)
	{
		bytes_read = read(cgi.pipe_out[0], buffer, MESSAGE_BUFFER * 2);

		if (bytes_read > 0)
		{
			// Append data to response
			client.response.response_content.append(buffer, bytes_read);
			client.updateActivity();
			// Clear buffer (C++98 compatible - no memset)
			for (size_t i = 0; i < sizeof(buffer); ++i)
				buffer[i] = 0;

			// If we have data and response starts with HTTP headers, we can start sending
			// Check if we have at least the status line and headers (look for \r\n\r\n)
			size_t header_end = client.response.response_content.find("\r\n\r\n");
			if (header_end != std::string::npos)
			{
				// Headers are complete, we can start sending
				// Update write_buffer if we're not already sending
				if (client.write_offset == 0)
				{
					// Ensure we have HTTP status line
					if (client.response.response_content.find("HTTP/1.1") == std::string::npos)
					{
						client.response.response_content.insert(0, "HTTP/1.1 200 OK\r\n");
					}
					// Update write_buffer with current response_content
					client.write_buffer = client.response.getRes();

					// Add to write_set if not already there
					if (!_fd_manager.isSet(client_fd, _write_set))
					{
						_fd_manager.add(client_fd, _write_set);
						Logger::info("CGI headers complete, starting to send response for fd=" + toString(client_fd));
					}
				}
				else
				{
					// Already sending, but update buffer with new data
					// This is important: the CGI script may write more data after we started sending
					std::string new_buffer = client.response.getRes();
					if (new_buffer.size() > client.write_buffer.size())
					{
						// More data arrived, update buffer
						// Keep the current offset so we don't resend what we already sent
						client.write_buffer = new_buffer;

						// Always ensure client is in write_set when we have new data
						if (!_fd_manager.isSet(client_fd, _write_set))
						{
							_fd_manager.add(client_fd, _write_set);
						}
					}
				}
			}

			// Continue reading to get all available data
			continue;
		}
		else if (bytes_read == 0)
		{
			// EOF: CGI finished writing
			_fd_manager.remove(cgi.pipe_out[0], _read_set);
			close(cgi.pipe_out[0]);

			// Wait for CGI process to finish (non-blocking check first)
			int status;
			pid_t wait_result = waitpid(cgi.getCgiPid(), &status, WNOHANG);

			// If process hasn't finished yet, wait for it (should be quick since pipe is closed)
			if (wait_result == 0)
			{
				waitpid(cgi.getCgiPid(), &status, 0);
			}

			if (wait_result > 0 && WIFEXITED(status) && WEXITSTATUS(status) != 0)
			{
				client.response.setErrorResponse(502);
			}

			client.response.setCgiState(2);

			// If response doesn't start with HTTP/1.1, add status line
			if (client.response.response_content.find("HTTP/1.1") == std::string::npos)
			{
				client.response.response_content.insert(0, "HTTP/1.1 200 OK\r\n");
			}

			// Update write_buffer with final response
			std::string final_buffer = client.response.getRes();
			client.write_buffer = final_buffer;

			// Check if we need to send more data
			if (client.write_offset < final_buffer.size())
			{
				// There's still data to send
				// Keep current offset - we've already sent up to that point

				// Ensure client is in write_set to continue sending
				if (!_fd_manager.isSet(client_fd, _write_set))
				{
					_fd_manager.add(client_fd, _write_set);
				}
			}
			else
			{
				// All data already sent - close connection immediately
				// This ensures browsers receive the complete response
				Logger::info("CGI response complete for fd=" + toString(client_fd) + " (size: " + toString(client.write_buffer.size()) + " bytes) - closing connection");
				_fd_manager.remove(client_fd, _write_set);
				closeClient(client_fd);
				return;
			}

			Logger::info("CGI response complete for fd=" + toString(client_fd) + " (size: " + toString(client.write_buffer.size()) + " bytes)");
			return;
		}
		else // bytes_read < 0
		{
			client.updateActivity();
			return;
		}
	}
}

