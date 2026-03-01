#pragma once
#include <atomic>
#define _WIN32_WINNT 0x0A00
#include <boost/asio.hpp>
#include <memory>
#include <thread>

using boost::asio::ip::tcp;

class TcpEchoServer
{
	public:
	TcpEchoServer();
	~TcpEchoServer();

	bool Start(uint16_t port = 0);
	void Stop();
	uint16_t GetPort() const;

	private:
	boost::asio::io_context io_context_;
	std::unique_ptr<tcp::acceptor> acceptor_;

	std::atomic<bool> is_running_{ false };
	uint16_t bound_port_ = 0;

	void AcceptLoop();

	// Asio sockets are moveable
	void HandleClient(tcp::socket client_socket);
};