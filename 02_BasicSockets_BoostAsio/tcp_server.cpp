#include "tcp_server.h"
#include <iostream>

TcpEchoServer::TcpEchoServer() = default;

TcpEchoServer::~TcpEchoServer()
{
	Stop();
}

bool TcpEchoServer::Start(uint16_t port)
{
	try
	{
		// 1. Create an endpoint (IP address and port). tcp::v4() = any IPv4 address
		tcp::endpoint endpoint(tcp::v4(), port);

		// 2. The acceptor automatically opens the socket, binds it, and starts listening
		acceptor_ = std::make_unique<tcp::acceptor>(io_context_, endpoint);

		// 3. Retrieve the dynamically assigned port
		bound_port_ = acceptor_->local_endpoint().port();
		is_running_ = true;

		// 4. Enter the blocking accept loop
		AcceptLoop();
		return true;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Server Start failed: " << e.what() << "\n";
		return false;
	}
}

void TcpEchoServer::Stop()
{
	is_running_ = false;
	if (acceptor_ && acceptor_->is_open())
	{
		// Closing the acceptor forces any blocking accept() call to throw/return
		boost::system::error_code ec;
		acceptor_->close(ec);
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
		try
		{
			// Create a new socket for the incoming client
			tcp::socket client_socket(io_context_);

			// This blocks until the 3-way handshake is complete
			acceptor_->accept(client_socket);

			// Spawn a thread and move the socket into it
			std::thread client_thread([this, socket = std::move(client_socket)]() mutable
				{
					HandleClient(std::move(socket));
				});
			client_thread.detach();
		}
		catch (const boost::system::system_error& e)
		{
			if (is_running_)
			{
				std::cerr << "Accept failed: " << e.what() << "\n";
			}
		}
	}
}

void TcpEchoServer::HandleClient(tcp::socket client_socket)
{
	try
	{
		char buffer[1024];
		boost::system::error_code error;

		while (true)
		{
			// Read some data (blocking)
			size_t bytes_received = client_socket.read_some(boost::asio::buffer(buffer), error);

			if (error == boost::asio::error::eof)
			{
				break; // Connection closed cleanly by peer
			}
			else if (error)
			{
				throw boost::system::system_error(error); // Some other error
			}

			// Echo the data back
			boost::asio::write(client_socket, boost::asio::buffer(buffer, bytes_received));
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Client error: " << e.what() << "\n";
	}
	// No need to call closesocket(), the tcp::socket destructor handles it
}