/**
 * ConnectedClient.cpp
 *
 * Implementation file for ConnectedClient class.
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <span>
#include <string>
#include <memory>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "networking.h"
#include "protocol.h"
#include "ConnectedClient.h"
#include "ChunkedDataSender.h"
#include "SongLibrary.h"
#include <cstring> //memcpy

using std::cout;
using std::cerr;
using std::unique_ptr;
using std::vector;
using std::array;
using std::string;
using std::span;


ConnectedClient::ConnectedClient(int fd, ClientState initial_state, const SongLibrary* library) :
	client_fd(fd), state(initial_state), song_library(library){}

void ConnectedClient::send_with_sender(int epoll_fd, std::unique_ptr<ChunkedDataSender> chunk_sender) {
	ssize_t num_bytes_sent;
	ssize_t total_bytes_sent = 0;

	// keep sending the next chunk until it says we either didn't send
	// anything (0 return indicates nothing left to send) or until we can't
	// send anymore because of a full socket buffer (-1 return value)
	while((num_bytes_sent = chunk_sender->send_next_chunk(this->client_fd)) > 0) {
		total_bytes_sent += num_bytes_sent;
	}
	cout << "sent " << total_bytes_sent << " bytes to client\n";

	if (num_bytes_sent < 0) {
		// Fill this in with the three steps listed in the comment above.
		this->state = SENDING;
		this->sender = std::move(chunk_sender);
		modify_socket_in_epoll(this->client_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP, epoll_fd);
	}
}

void ConnectedClient::send_message(int epoll_fd, span<char> data_to_send) {
	// Creates a new ArraySender in heap memory. The unique_ptr means that
	// this function owns that memory and therefore no one else can have a
	// pointer to it.
	std::unique_ptr<ChunkedDataSender> array_sender = std::make_unique<ArraySender>(data_to_send);
	send_with_sender(epoll_fd, std::move(array_sender));
}

void ConnectedClient::send_message(int epoll_fd, const std::filesystem::path& file_path) {
	// Creates a new ArraySender in heap memory. The unique_ptr means that
	// this function owns that memory and therefore no one else can have a
	// pointer to it.
	std::unique_ptr<ChunkedDataSender> file_sender = std::make_unique<FileSender>(file_path);
	send_with_sender(epoll_fd, std::move(file_sender));
}

void ConnectedClient::handle_input(int epoll_fd) {
	cout << "Ready to read from client " << this->client_fd << "\n";
	array<char, 1024> data;
	ssize_t bytes_received = recv(this->client_fd, data.data(), data.size(), 0);
	if (bytes_received < 0) {
		perror("client_read recv");
		exit(EXIT_FAILURE);
	}

	// We likely didn't receive enough bytes to fill up the data array, so
	// let's create a span that covers only the parts of data that were filled
	// in by recv.
	span<char> undecoded(data.begin(), data.begin()+bytes_received);

	// Create a header from the data we just received
	Header received_header(undecoded);
	cerr << "message_type = " << static_cast<int>(received_header.get_type()) << "\n";
	cerr << "payload_len = " << received_header.get_len() << "\n";

	// Convert the message payload to a string (so we can print it)
	// @note: For project 5, you'll use the unpack function (from
	// networking.h) if your payload is numerical rather than text-based
	std::string payload_val(undecoded.begin()+Header::size, undecoded.end());
	cout << "Received payload (" << payload_val.size() << " bytes): " << payload_val << "\n";


	// Create the response message to send to the client.
	// @note This is a contrived example that sends a whole bunch of 'u'
	// characters if the client send "hello", a whole bunch of 'x'
	// characters if they said "hola", and an error message if they sent any
	// other message.
	// In project 5, you'll have to customize your response based on the
	// specifics of your protocol.
	vector<char> message;
	message_type response_type;

	if (received_header.get_type() == INFO_REQUEST) {
		//send the info for songID from payload
		uint32_t songID = 0;
		if (received_header.get_len() >=4){
			std::memcpy(&songID, undecoded.data() +Header::size, sizeof(uint32_t));
			songID = ntohl(songID);
		}
		
		//Look up file path info
		std::optional<std::filesystem::path> info_path = song_library->get_info_file_path(songID);
		if (info_path.has_value()){
			std::ifstream info_file(info_path.value());
			std::stringstream buffer;
			buffer << info_file.rdbuf();
			string info_response = buffer.str();
			message.assign(info_response.begin(), info_response.end());
			response_type = INFO_RESPONSE;
		} else{
			//song exists so we check if there is an info file
			std::optional<std::filesystem::path> info_path = song_library->get_info_file_path(songID);
			if (info_path.has_value()) {
				std::ifstream info_file(info_path.value());
				std::stringstream buffer;
				buffer << info_file.rdbuf();
				string info_response = buffer.str();
				message.assign(info_response.begin(), info_response.end());
				response_type = INFO_RESPONSE;
			} else {
				//song exists but it doesnt have an info file
				string noInfo = "No info available for this song";
				message.assign(noInfo.begin(), noInfo.end());
				response_type = INFO_RESPONSE;
			}
		}
	}
	else if (received_header.get_type() == PLAY_REQUEST) {
		//Parse songID from payload 4 byte int
		uint32_t songID = 0;
		if (received_header.get_len()>=4){
			std::memcpy(&songID, undecoded.data() + Header::size, sizeof(uint32_t));
			songID = ntohl(songID);
		}
		//Look up song File Path
		std::optional<std::filesystem::path> song_path = song_library->get_song_file_path(songID);
		if (song_path.has_value()){
			size_t file_size = std::filesystem::file_size(song_path.value());

			//send the header with file size
			Header h(PLAY_RESPONSE, file_size);
			std::vector<char> header_bytes = h.to_byte_array();
			this->send_message(epoll_fd, header_bytes);

			//send the actual file
			this->send_message(epoll_fd, song_path.value());
		} else{
			//Send Error Response
			string errorMsge = "Invalid songID";
			Header h(ERROR_RESPONSE, errorMsge.size());
			std::vector<char> header_bytes = h.to_byte_array();
			this->send_message(epoll_fd, header_bytes);

			vector<char> error_vec(errorMsge.begin(), errorMsge.end());
			this->send_message(epoll_fd, error_vec);
		}
		return;
	}
	else if (received_header.get_type() == LIST_REQUEST){
		string list_response = song_library->get_song_list();
		message.assign(list_response.begin(), list_response.end());
		response_type = LIST_RESPONSE;
	}
	else if (received_header.get_type() == STOP_REQUEST) {
		//stop sending if we are currently sending
		if (this->state == SENDING) {
			this->state = RECEIVING;
			this->sender.reset();
			modify_socket_in_epoll(this->client_fd, EPOLLIN | EPOLLRDHUP, epoll_fd);
		}
		
		//tell the client that we're done sending
		shutdown(this->client_fd, SHUT_WR);
		return;
	}
	else {
		//Unknown Request, send an error.
		string error_msge = "Unknown Request Type";
		message.assign(error_msge.begin(), error_msge.end());
		response_type = ERROR_RESPONSE;
	}

	// Create and send message header back to the client
	Header h(response_type, message.size());
	std::vector<char> header_bytes = h.to_byte_array();
	this->send_message(epoll_fd, header_bytes);

	// Send the message now
	this->send_message(epoll_fd, message);
}


// You likely should not need to modify this function.
void ConnectedClient::handle_close(int epoll_fd) {
	cout << "Closing connection to client " << this->client_fd << "\n";
	remove_socket_from_epoll(this->client_fd, epoll_fd);
	close(this->client_fd);
}

void ConnectedClient::resume_sending(int epoll_fd) {
	ssize_t num_bytes_sent;
	ssize_t total_bytes_sent = 0;

	// keep sending the next chunk until it says we either didn't send
	// anything (0 return indicates nothing left to send) or until we can't
	// send anymore because of a full socket buffer (-1 return value)
	while((num_bytes_sent = this->sender->send_next_chunk(this->client_fd)) > 0) {
		total_bytes_sent += num_bytes_sent;
	}
	cout << "sent " << total_bytes_sent << " bytes to client\n";

	if (num_bytes_sent < 0) {
		// Fill this in with the three steps listed in the comment above.
        this->state = RECEIVING;
        this->sender.reset();
        modify_socket_in_epoll(this->client_fd, EPOLLIN | EPOLLRDHUP, epoll_fd);
	}
}