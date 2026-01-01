/**
 * File: ClientSocket.cpp
 *
 * Implementation of ClientSocket class.
 *
 * Author: Sat Garcia (sat@sandiego.edu)
 */

// operating system specific libraries
#include <unistd.h>
#include <sys/socket.h>

// C++ standard libraries
#include <span>
#include <vector>
#include <system_error>

using std::vector;
using std::span;

#include "ClientSocket.hpp"

void ClientSocket::close() { ::close(this->socket_fd); }

void ClientSocket::sendData(span<const char> data) {
	size_t total_bytes_sent = 0; // start at the beginning of the data span
	size_t total_bytes_we_needed_to_send = data.size();

	// loop to send data in chunks until it has all been sent 
	while(total_bytes_sent < total_bytes_we_needed_to_send) {
		// send as much data as possible at once
		int num_bytes_sent = send(
			this->socket_fd, 
			data.data() + total_bytes_sent, // starting index of current chunk of data to be sent
			total_bytes_we_needed_to_send - total_bytes_sent, // remaining size of data that needs to be sent
			0
		);
		
		if (num_bytes_sent == -1) {
			std::error_code ec(errno, std::generic_category());
			throw std::system_error(ec, "send failed");
		}
		
		// count how much data has actually been sent
		total_bytes_sent += num_bytes_sent;
	}
}

vector<char> ClientSocket::receiveData(size_t max_size) {
	vector<char> data(max_size, '\0');

	int num_bytes_received = recv(this->socket_fd, data.data(), max_size, 0);
	if (num_bytes_received == -1) {
		std::error_code ec(errno, std::generic_category());
		throw std::system_error(ec, "recv failed");
	}

	data.resize(num_bytes_received);

	return data;
}
