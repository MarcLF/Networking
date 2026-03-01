#include "gtest/gtest.h"
#include "tcp_server.h"
#include <thread>
#include <vector>

// Linking the library ws2_32
#pragma comment(lib, "ws2_32.lib")

class WinsockTestFixture : public ::testing::Test
{
	protected:
	void SetUp() override
	{
		WSADATA wsa_data;
		// Initialize Winsock version 2.2
		int result = ::WSAStartup(MAKEWORD(2, 2), &wsa_data);
		ASSERT_EQ(result, 0) << "WSAStartup failed: " << result;
	}

	void TearDown() override
	{
		::WSACleanup();
	}
};

TEST_F(WinsockTestFixture, EchoServerReceivesAndSendsData)
{
	TcpEchoServer server;

	// 1 Start the server on a background thread using an ephemeral port
	std::thread server_thread([&server]()
		{
			server.Start(0);
		});

	// Give the server some time to bind and listen
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	uint16_t server_port = server.GetPort();
	ASSERT_GT(server_port, 0) << "Server failed to bind to a valid port.";

	// 2 Create a Client Socket on the main test thread
	SOCKET client_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	ASSERT_NE(client_socket, INVALID_SOCKET);

	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

	// 3 Connect to the Server, three way handshake, blocking call
	int connect_result = ::connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr));
	ASSERT_EQ(connect_result, 0) << "Client failed to connect to server.";

	// 4 Send a message
	std::string test_message = "Hello!";
	int bytes_sent = ::send(client_socket, test_message.c_str(), test_message.length(), 0);
	ASSERT_EQ(bytes_sent, test_message.length());

	// 5 Receive the echo
	std::vector<char> buffer(1024);
	int bytes_received = ::recv(client_socket, buffer.data(), buffer.size(), 0);
	ASSERT_GT(bytes_received, 0);

	std::string received_message(buffer.data(), bytes_received);
	EXPECT_EQ(received_message, test_message);

	// 6 Cleanup
	closesocket(client_socket);
	server.Stop();
	if (server_thread.joinable())
	{
		server_thread.join();
	}
}