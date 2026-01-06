/**
 * @file main.cpp
 * @brief WebServ Network Layer - Entry Point
 * 
 * COMPLETE FLOW EXAMPLE: Browser requests /banana.jpg
 * 
 * 1. STARTUP (main):
 *    - Load config → create server socket fd=5 on port 8080
 *    - Add fd=5 to _read_set
 *    - Enter event loop (select)
 * 
 * 2. CONNECTION (acceptNewConnection):
 *    - Browser connects → select() says fd=5 readable
 *    - accept(5) → creates client socket fd=10
 *    - Add fd=10 to _read_set
 * 
 * 3. REQUEST (handleClientRead):
 *    - Browser sends "GET /banana.jpg HTTP/1.1\r\n\r\n"
 *    - select() says fd=10 readable
 *    - recv(10) → read into client.read_buffer
 *    - Parse HTTP request
 *    - Build response (read banana.jpg file)
 *    - Add fd=10 to _write_set
 * 
 * 4. RESPONSE (handleClientWrite):
 *    - select() says fd=10 writable
 *    - send(10, "HTTP/1.1 200 OK\r\n...[JPEG data]")
 *    - May take multiple calls if large file
 *    - Track progress with write_offset
 * 
 * 5. CLEANUP (closeClient):
 *    - All data sent → close(10)
 *    - Remove fd=10 from all sets
 *    - Free memory
 * 
 * Person A's responsibility: Network & Client Management
 * Person B's responsibility: HTTP Logic (replaces SimpleResponse)
 */

#include "ServerManager.hpp"
#include "Logger.hpp"
#include <csignal>

static ServerManager* g_manager = NULL;

void signalHandler(int signum)
{
	(void)signum;
	Logger::info("Received signal, shutting down...");
	if (g_manager)
		g_manager->stop();
}

void setupSignalHandlers()
{
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	signal(SIGPIPE, SIG_IGN);
}

int main(int argc, char** argv)
{
	std::string config_file = "config/default.conf";
	
	if (argc > 1)
		config_file = argv[1];
	
	Logger::info("Starting WebServ...");
	Logger::info("Config file: " + config_file);
	setupSignalHandlers();
	
	try
	{
		ServerManager manager;
		g_manager = &manager;
		
		manager.loadConfig(config_file);
		Logger::info("Server ready - starting event loop");
		manager.run();
	}
	catch (const std::exception& e)
	{
		Logger::error(std::string("Fatal error: ") + e.what());
		return 1;
	}
	
	Logger::info("WebServ shutdown complete");
	return 0;
}

