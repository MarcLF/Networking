#pragma once
#include <array>
#define _WIN32_WINNT 0x0A00
#include <boost/asio.hpp>
#include <memory>

using boost::asio::ip::tcp;

// Represents a single active client connection
class TcpSession : public std::enable_shared_from_this<TcpSession>
{
	public:
	explicit TcpSession(tcp::socket socket);
	void Start();

	private:
	tcp::socket socket_;
	std::array<char, 1024> buffer_{};

	void DoRead();
	void DoWrite(std::size_t bytes_transferred);
};

// Represents the server that accepts incoming connections
class TcpAsyncServer
{
	public:
	TcpAsyncServer(uint16_t port = 0);
	~TcpAsyncServer();

	void Run();   // Blocks and processes async events
	void Stop();  // Stops the event loop
	uint16_t GetPort() const;

	private:
	boost::asio::io_context io_context_;
	tcp::acceptor acceptor_;
	uint16_t bound_port_ = 0;

	void DoAccept();
};