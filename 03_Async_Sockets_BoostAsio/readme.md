# Boost.Asio Asynchronous TCP Echo Server

This project introduces the high-performance architecture used in modern finance and trading systems. It abandons the "thread-per-connection" model in favor of the Proactor pattern, allowing a single thread to juggle thousands of concurrent connections without ever blocking.

1. Initializes a Boost `io_context` and a `tcp::acceptor` bound to an ephemeral port.
2. The server posts an `async_accept` request to the OS and *immediately returns*, leaving the thread completely unblocked.
3. The background thread executes `io_context.run()`, which acts as an event loop waiting for OS completion callbacks (IOCP on Windows).
4. The test client connects to the server, prompting the OS to fire the accept callback and create a new `TcpSession`.
5. The `TcpSession` secures its own memory via `std::shared_ptr` and posts an `async_read_some` request.
6. The client sends a message, triggering the server's read callback, which immediately chains into an `async_write` callback to echo the data.
7. The client synchronously receives the echoed message and validates it.
8. The client disconnects, triggering an EOF error in the server's event loop, which gracefully destroys the isolated `TcpSession`.