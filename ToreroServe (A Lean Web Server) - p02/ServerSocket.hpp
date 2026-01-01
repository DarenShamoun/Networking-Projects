#ifndef SERVERSOCKET_HPP
#define SERVERSOCKET_HPP

/**
 * File: ServerSocket.hpp
 *
 * Header file for ServerSocket class.
 *
 * DO NOT MODIFY THIS FILE IN ANY WAY!
 *
 * Author: Sat Garcia (sat@sandiego.edu)
 */

class ClientSocket; // forward declaration

class ServerSocket {
	public:
		/*
		 * Creates socket that will be bound to the given port number.
		 */
		ServerSocket(unsigned short int port_num);
		
		// destructor (closes socket)
		~ServerSocket();

		// copy constructor and assignment methods
		ServerSocket(const ServerSocket&) = delete;
		void operator=(const ServerSocket&) = delete;

		// move constructor
		ServerSocket(ServerSocket&& other) : socket_fd{other.socket_fd} {
			other.socket_fd = -1;
		}

		// move assignment operator (swap)
		ServerSocket& operator=(ServerSocket&& other);

		/**
		 * Starts listening for incoming connections.
		 */
		void startListening();

		/**
		 * Accept the next client.
		 *
		 * @return The newly connected client
		 */
		ClientSocket acceptConnection();

	private:
		int socket_fd;
		unsigned short int port_num;
};
#endif
