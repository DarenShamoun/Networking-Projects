/*
 * File: UnreliableSocket.cpp
 *
 * Implementation of socket using UDP.
 *
 */

// C++ library includes
#include <iostream>
#include <array>

// OS specific includes
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "UnreliableSocket.h"
#include "rdt_time.h"
#include "aixlog.hpp"

using std::array;

/*
 * NOTE: Function header comments shouldn't go in this file: they should be put
 * in the ReliableSocket header file.
 */

UnreliableSocket::UnreliableSocket() {
	this->timeout_length = 0;
	this->connected = false;

	this->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (this->sock_fd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
}

void UnreliableSocket::bind(int port_num) {
	// Bind specified port num using our local IPv4 address.
	// This allows remote hosts to connect to a specific port.
	struct sockaddr_in addr; 
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_num);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (::bind(this->sock_fd, (struct sockaddr*)&addr, sizeof(addr))) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
}

void UnreliableSocket::connect(char *hostname, int port_num) {
	// set up IPv4 address info with given hostname and port number
	struct sockaddr_in addr; 
	addr.sin_family = AF_INET; 	// use IPv4
	addr.sin_addr.s_addr = inet_addr(hostname);
	addr.sin_port = htons(port_num); 

	/*
	 * UDP isn't connection-oriented, but calling connect here allows us to
	 * remember the remote host (stored in fromaddr).
	 * This means we can then use send and recv instead of the more complex
	 * sendto and recvfrom.
	 */
	if(::connect(this->sock_fd, (struct sockaddr*)&addr, sizeof(addr))) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	this->connected = true;
}


uint32_t UnreliableSocket::get_timeout_length() {
	return this->timeout_length;
}


void UnreliableSocket::set_timeout_length(uint32_t timeout_length_ms) {
	LOG(INFO) << "Setting timeout to " << timeout_length_ms << " ms\n";
	this->timeout_length = timeout_length_ms;

	struct timeval timeout;
	msec_to_timeval(timeout_length_ms, &timeout);

	if (setsockopt(this->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
					sizeof(struct timeval)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
}


void UnreliableSocket::send(const void *buffer, int length) {
	if (!this->connected) {
		LOG(WARNING) << "Cannot send: Connection not established.\n";
		return;
	}

	LOG(DEBUG) << "Unreliable send of " << length << " bytes of data\n";

	if (::send(this->sock_fd, buffer, length, 0) < 0) {
		perror("UnreliableSocket::send");
		exit(EXIT_FAILURE);
	}
}


int UnreliableSocket::receive(array<uint8_t, MAX_SEG_SIZE>& buffer) {
	int num_bytes_received = -1;

	struct sockaddr_in fromaddr;
	unsigned int addrlen = sizeof(fromaddr);
	if (!this->connected) {
		// if we aren't "connected", we need to use the more complicated
		// recvfrom function.
		num_bytes_received = recvfrom(this->sock_fd, buffer.data(), MAX_SEG_SIZE, 0, 
								(struct sockaddr*)&fromaddr, &addrlen);		
	}
	else {
		num_bytes_received = recv(this->sock_fd, buffer.data(), MAX_SEG_SIZE, 0);
	}

	if (num_bytes_received < 0) {
		if (errno == EAGAIN) {
			// this was a timeout
			throw SocketTimeout();
		}
		else {
			perror("recv");
			exit(EXIT_FAILURE);
		}
	}
	else if (num_bytes_received == 0) {
		throw ConnectionClosed();
	}

	if (!connected) {
		// If we weren't "connected", do that now.
		
		/*
		 * UDP isn't connection-oriented, but calling connect here allows us to
		 * remember the remote host (stored in fromaddr).
		 * This means we can then use send and recv instead of the more complex
		 * sendto and recvfrom.
		 */
		if (::connect(this->sock_fd, (struct sockaddr*)&fromaddr, addrlen)) {
			perror("accept connect");
			exit(EXIT_FAILURE);
		}

		this->connected = true;
	}

	return num_bytes_received;
}


void UnreliableSocket::close() {
	if (::close(this->sock_fd) < 0) {
		perror("close_connection close");
	}
}
