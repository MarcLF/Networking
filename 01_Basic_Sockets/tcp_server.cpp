#include "tcp_server.h"
#include <iostream>
#include <thread>

TcpEchoServer::TcpEchoServer() : listen_socket_(INVALID_SOCKET), is_running_(false), bound_port_(0) {}

TcpEchoServer::~TcpEchoServer()
{
	Stop();
}

bool TcpEchoServer::Start(uint16_t port)
{
	// 1 Create the listening socket
	listen_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket_ == INVALID_SOCKET)
	{
		std::cerr << "Failed to create socket. Error: " << WSAGetLastError() << "\n";
		return false;
	}

	// 2 Prepare the sockaddr_in structure
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
	server_addr.sin_port = htons(port);       // Convert port to network byte order

	// 3 Bind the socket
	if (bind(listen_socket_, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		std::cerr << "Bind failed. Error: " << WSAGetLastError() << "\n";
		closesocket(listen_socket_);
		return false;
	}

	// 4 If port 0 was passed, the OS assigned a dynamic port. We need to retrieve it.
	int addr_len = sizeof(server_addr);
	if (getsockname(listen_socket_, (sockaddr*)&server_addr, &addr_len) == 0)
	{
		bound_port_ = ntohs(server_addr.sin_port); // Convert back to host byte order
	}

	// 5 Start listening for incoming connections
	if (listen(listen_socket_, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Listen failed. Error: " << WSAGetLastError() << "\n";
		closesocket(listen_socket_);
		return false;
	}

	is_running_ = true;

	// 6 Enter the blocking accept loop
	AcceptLoop();

	return true;
}

void TcpEchoServer::Stop()
{
	is_running_ = false;
	if (listen_socket_ != INVALID_SOCKET)
	{
		// Closing the socket will force the blocking accept() call in AcceptLoop to return
		closesocket(listen_socket_);
		listen_socket_ = INVALID_SOCKET;
	}
}

uint16_t TcpEchoServer::GetPort() const
{
	return bound_port_;
}

void TcpEchoServer::AcceptLoop()
{
	while (is_running_)
	{
		sockaddr_in client_addr{};
		int client_addr_len = sizeof(client_addr);

		// This call blocks until a client connects (or the socket is closed by Stop())
		SOCKET client_socket = ::accept(listen_socket_, (sockaddr*)&client_addr, &client_addr_len);

		if (client_socket == INVALID_SOCKET)
		{
			if (is_running_)
			{
				std::cerr << "Accept failed. Error: " << WSAGetLastError() << "\n";
			}
			continue; // If not running, the loop will exit naturally
		}

		// Spawn a detached thread to handle this specific client so the server 
		// can immediately go back to accepting new connections.
		std::thread client_thread([this, client_socket]()
			{
				HandleClient(client_socket);
			});
		client_thread.detach();
	}
}

void TcpEchoServer::HandleClient(SOCKET client_socket)
{
	char buffer[1024];
	int bytes_received;

	// Keep reading until the client disconnects or an error occurs
	while (true)
	{
		bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

		if (bytes_received > 0)
		{
			// Echo the exact same bytes back to the client
			int bytes_sent = send(client_socket, buffer, bytes_received, 0);
			if (bytes_sent == SOCKET_ERROR)
			{
				std::cerr << "Send failed. Error: " << WSAGetLastError() << "\n";
				break;
			}
		}
		else if (bytes_received == 0)
		{
			// Client gracefully closed the connection
			break;
		}
		else
		{
			// Error occurred (client forcefully disconnected, etc.)
			std::cerr << "Recv failed. Error: " << WSAGetLastError() << "\n";
			break;
		}
	}

	// Clean up the client socket when done
	closesocket(client_socket);
}