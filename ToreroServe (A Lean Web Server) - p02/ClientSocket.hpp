#ifndef CLIENTSOCKET_HPP
#define CLIENTSOCKET_HPP

/**
 * File: ClientSocket.hpp
 *
 * Header file for ClientSocket class.
 *
 * DO NOT MODIFY THIS FILE IN ANY WAY!
 *
 * Author: Sat Garcia (sat@sandiego.edu)
 */

#include <span>
#include <vector>

class ClientSocket {
	public:
		// constructor
		ClientSocket(int socket_fd) : socket_fd(socket_fd) {};

		void close();

		/**
		 * Sends message over this socket, raising an exception if there was a problem
		 * sending.
		 *
		 * @param data The data to send.
		 */
		void sendData(std::span<const char> data);

		/**
		 * Receives message over the client's socket, raising an exception if there
		 * was an error in receiving.
		 *
		 * @return Vector of char values read in from the socket.
		 */
		std::vector<char> receiveData(size_t max_size);

	private:
		int socket_fd;
};
#endif
