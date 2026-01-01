#include <iostream>
#include <algorithm>
#include <utility>
#include <span>
#include <array>

#include <cstring>
#include <cerrno>
#include <cstdio>

#include <sys/types.h>
#include <sys/socket.h>

#include "ChunkedDataSender.h"

ArraySender::ArraySender(std::span<char> array_to_send) : array(array_to_send.begin(), array_to_send.end()), curr_loc(0) {}

ssize_t ArraySender::send_next_chunk(int sock_fd) {
	// Determine how many bytes we need to put in the next chunk.
	// This will be either the CHUNK_SIZE constant or the number of bytes left
	// to send in the array, whichever is smaller.
	size_t num_bytes_remaining = array.size() - curr_loc;
	size_t bytes_in_chunk = std::min(num_bytes_remaining, CHUNK_SIZE);

	if (bytes_in_chunk > 0) {
		// send next chunk of data in the array, starting at curr_loc offset
		ssize_t num_bytes_sent = send(sock_fd, &array[curr_loc], bytes_in_chunk, 0);

		if (num_bytes_sent > 0) {
			// We successfully send some of the data so update our location in
			// the array so we know where to start sending the next time we
			// call this function.
			curr_loc += num_bytes_sent;
			return num_bytes_sent;
		}
		else if (num_bytes_sent < 0 && errno == EAGAIN) {
			// We couldn't send anything because the buffer was full
			return -1;
		}
		else {
			// Send had an error which we didn't expect, so exit the program.
			perror("send_next_chunk send");
			exit(EXIT_FAILURE);
		}
	}
	else {
		return 0;
	}
}

FileSender::FileSender(const std::filesystem::path& file_path) : file(file_path, std::ios::binary), file_size(std::filesystem::file_size(file_path)), bytes_sent(0) {}

ssize_t FileSender::send_next_chunk(int sock_fd) {
	size_t num_bytes_remaining = file_size - bytes_sent;
	size_t bytes_in_chunk = std::min(num_bytes_remaining, CHUNK_SIZE);

	if (bytes_in_chunk > 0) {
		//read the chunk of data from the file
		std::array<char, CHUNK_SIZE> file_data;
		file.read(file_data.data(), bytes_in_chunk);
		size_t bytes_read = file.gcount();

		//send the chunk of data over the socket
		ssize_t num_bytes_sent = send(sock_fd, file_data.data(), bytes_read, 0);

		if (num_bytes_sent > 0) {
			bytes_sent += num_bytes_sent;

			//and if we havent sent everything yet, rewind the file
			if (static_cast<size_t>(num_bytes_sent) < bytes_read) {
				file.seekg(num_bytes_sent - bytes_read, std::ios::cur);
			}
			return num_bytes_sent;
		}
		else if(num_bytes_sent < 0) {
			//if we couldnt send then rewind and try again later
			file.seekg(static_cast<long>(bytes_read), std::ios::cur);
			return -1;
		}
		else {
			perror("FileSender error");
			exit(EXIT_FAILURE);
		}
	}
	else {
		return 0;
	}
}