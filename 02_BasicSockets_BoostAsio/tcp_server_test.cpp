#include "gtest/gtest.h"
#include "tcp_server.h"
#include <string>
#include <thread>
#include <vector>

TEST(BoostAsioTests, EchoServerReceivesAndSendsData)
{
	TcpEchoServer server;

	// 1. Start server on a background thread
	std::thread server_thread([&server]()
		{
			server.Start(0);
		});

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	uint16_t server_port = server.GetPort();
	ASSERT_GT(server_port, 0);

	// 2. Client setup using Asio
	boost::asio::io_context io_context;
	tcp::socket client_socket(io_context);

	// Connect to 127.0.0.1 on the server's port (throws on failure)
	tcp::endpoint server_endpoint(boost::asio::ip::make_address("127.0.0.1"), server_port);
	ASSERT_NO_THROW(client_socket.connect(server_endpoint));

	// 3. Send a message
	std::string test_message = "Hello via Boost!";
	size_t bytes_sent = boost::asio::write(client_socket, boost::asio::buffer(test_message));
	ASSERT_EQ(bytes_sent, test_message.length());

	// 4. Receive the echo
	std::vector<char> buffer(1024);
	boost::system::error_code error;
	size_t bytes_received = client_socket.read_some(boost::asio::buffer(buffer), error);

	ASSERT_FALSE(error);
	ASSERT_GT(bytes_received, 0);

	std::string received_message(buffer.data(), bytes_received);
	EXPECT_EQ(received_message, test_message);

	// 5. Cleanup
	client_socket.close();
	server.Stop();
	if (server_thread.joinable())
	{
		server_thread.join();
	}
}