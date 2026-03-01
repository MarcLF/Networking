# Raw Winsock Synchronous TCP Echo Server

This project demonstrates the fundamental OS-level socket API using Windows Winsock2. It implements a blocking, thread-per-connection TCP server to illustrate how the operating system manages network buffers and the TCP 3-way handshake.

1. Initializes the Windows networking stack using `WSAStartup`.
2. Creates a listening TCP server socket, binds it to an ephemeral port, and enters a listening state.
3. Spawns a background thread that blocks on the `accept()` system call, freezing until a connection arrives.
4. Connects a test client to the server via `connect()`, triggering the TCP 3-way handshake in the OS kernel.
5. Sends a string message from the client to the server using `send()`.
6. The server receives the data via `recv()` and echoes it back to the client.
7. The client reads the echoed message, verifies the data integrity, and closes the connection (`closesocket()`).
8. The server is signaled to stop, and the network stack is cleaned up via `WSACleanup`.