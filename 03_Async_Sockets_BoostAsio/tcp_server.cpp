#include "tcp_server.h"
#include <iostream>

// TcpSession Implementation
TcpSession::TcpSession(tcp::socket socket) : socket_(std::move(socket)) {}

void TcpSession::Start()
{
	DoRead();
}

void TcpSession::DoRead()
{
	// Capture "self" (a shared_ptr to this object) in the lambda.
	// This guarantees the session object stays alive in memory as long 
	// as the OS is still trying to read data
	auto self(shared_from_this());

	socket_.async_read_some(boost::asio::buffer(buffer_),
		[this, self](boost::system::error_code error, std::size_t bytes_transferred)
		{
			if (!error)
			{
				// Data read successfully, now asynchronously write it back.
				DoWrite(bytes_transferred);
			}
		});
}

void TcpSession::DoWrite(std::size_t bytes_transferred)
{
	auto self(shared_from_this());

	boost::asio::async_write(socket_, boost::asio::buffer(buffer_, bytes_transferred),
		[this, self](boost::system::error_code error, std::size_t /*bytes_transferred*/)
		{
			if (!error)
			{
				// Write complete, go back to waiting for more data.
				DoRead();
			}
		});
}

// TcpAsyncServer Implementation
TcpAsyncServer::TcpAsyncServer(uint16_t port)
	: acceptor_(io_context_, tcp::endpoint(tcp::v4(), port))
{

	bound_port_ = acceptor_.local_endpoint().port();

	// Kick off the first asynchronous accept
	DoAccept();
}

TcpAsyncServer::~TcpAsyncServer()
{
	Stop();
}

void TcpAsyncServer::DoAccept()
{
	acceptor_.async_accept(
		[this](boost::system::error_code error, tcp::socket new_socket)
		{
			if (!error)
			{
				// 1. Create a new Session on the heap
				// 2. Start it (kicks off the first async read)
				std::make_shared<TcpSession>(std::move(new_socket))->Start();
			}

			// Immediately go back to listening for the next connection
			if (acceptor_.is_open())
			{
				DoAccept();
			}
		});
}

void TcpAsyncServer::Run()
{
	// Blocks the thread and processes all the lambdas we registered above whenever the OS finishes an io task.
	io_context_.run();
}

void TcpAsyncServer::Stop()
{
	io_context_.stop();
}

uint16_t TcpAsyncServer::GetPort() const
{
	return bound_port_;
}