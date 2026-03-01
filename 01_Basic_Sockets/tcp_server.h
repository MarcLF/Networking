#pragma once
#include <atomic>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

class TcpEchoServer
{
	public:
	TcpEchoServer();
	~TcpEchoServer();

	// Starts the server on the specified port. 
	// Uses port 0 to let the OS assign an ephemeral port.
	bool Start(uint16_t port = 0);
	void Stop();

	// Returns the port the server is actually listening on
	uint16_t GetPort() const;

	private:
	SOCKET listen_socket_ = INVALID_SOCKET;
	std::atomic<bool> is_running_{ false };
	uint16_t bound_port_ = 0;

	// The blocking loop that accepts incoming client connections
	void AcceptLoop();

	// Handles reading/writing for a single client
	void HandleClient(SOCKET client_socket);
};