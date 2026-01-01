#ifndef __UNRELIABLESOCKET_H__
#define __UNRELIABLESOCKET_H__
/*
 * File: UnReliableSocket.h
 *
 * Unreliable transport via UDP.
 *
 */

// Two "custom" classes for exceptions
class SocketTimeout {};
class ConnectionClosed {};

/**
 * Class that represents a UDP socket.
 */
class UnreliableSocket {
public:
	/*
	 * You probably shouldn't add any more public members to this class.
	 * Any new functions or fields you need to add should be private.
	 */
	// These are constants for all reliable connections
	static const int MAX_SEG_SIZE  = 1400; // FIXME: remove this?
	
	/**
	 * Basic Constructor, setting estimated RTT to 100 and deviation RTT to 10.
	 */
	UnreliableSocket();

	/**
	 * Binds this socket to the given port.
	 *
	 * @param port_num The port number to bind.
	 */
	void bind(int port_num);

	/**
	 * Connects this socket to the given host at the given port number.
	 *
	 * @param hostname The name of the remote host.
	 * @param port_num The port number on the remote host.
	 */
	void connect(char *hostname, int port_num);

	/**
	 * Send data to a remote host.
	 *
	 * @param buffer The buffer with data to be sent.
	 * @param length The amount of data in the buffer to send.
	 */
	void send(const void *buffer, int length);

	/**
	 * Receives data from remote host.
	 *
	 * @raises SocketTimeout If data not received before the specified
	 * timeout length.
	 * @raises ConnectionClosed If the connection was already been closed.
	 *
	 * @param buffer The buffer where received data will be stored.
	 * @return The amount of data actually received.
	 */
	int receive(std::array<uint8_t, MAX_SEG_SIZE>& buffer);

	/**
	 * Closes the socket.
	 */
	void close();

	/**
	 * Gets the current length of this socket's timeout, in milliseconds. Note
	 * that 0 represents infinite timeout length.
	 *
	 * @return The current timeout length (in ms)
	 */
	uint32_t get_timeout_length();

	/**
	 * Sets the timeout length of this socket, in milliseconds.
	 *
	 * @note Setting this to 0 makes the timeout length indefinite (i.e. could
	 * wait forever for a message).
	 *
	 * @param timeout_length_ms Length of timeout period in milliseconds.
	 */
	void set_timeout_length(uint32_t timeout_length_ms);

private:
	// Private member variables are initialized in the constructor
	int sock_fd;
	uint32_t timeout_length;
	bool connected;
};
#endif
