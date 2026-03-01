# Boost.Asio Synchronous TCP Echo Server

This project modernizes the raw OS socket calls by migrating to Boost.Asio. It demonstrates how to wrap legacy C-style networking into exception-safe, RAII-compliant C++ objects while maintaining a synchronous, blocking architecture.

1. Initializes a Boost `io_context` to handle underlying OS-level I/O operations automatically.
2. Creates a `tcp::acceptor` which abstracts away `socket()`, `bind()`, and `listen()` into a single object.
3. Spawns a background thread where the server blocks on `acceptor.accept()`, waiting for incoming clients.
4. Connects a Boost `tcp::socket` client to the server's endpoint, natively handling endianness and IP resolution.
5. Sends a message from the client using the `boost::asio::write` function.
6. The server reads the message synchronously and echoes it back over the wire.
7. The client reads the echoed message, asserts correctness, and safely closes the socket.
8. The server shuts down, relying on C++ destructors to cleanly release all OS socket handles without memory leaks.