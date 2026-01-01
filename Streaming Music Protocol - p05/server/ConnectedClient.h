#ifndef __CONNECTEDCLIENT_H__
#define __CONNECTEDCLIENT_H__
/**
 * ConnectedClient.cpp
 *
 * Header file for ConnectedClient class.
 */

#include <memory>
#include <span>
#include <filesystem>
#include "ChunkedDataSender.h"
#include "SongLibrary.h"
/**
 * Represents the state of a connected client.
 */
enum ClientState { RECEIVING, SENDING };

/**
 * Class that models a connected client.
 * 
 * One object of this class will be created for every client that you accept a
 * connection from.
 */
class ConnectedClient {
  private:
	// Private member Variablesa (i.e. fields)
	int client_fd;
	std::unique_ptr<ChunkedDataSender> sender;
	ClientState state;
	const SongLibrary* song_library;
	/** 
	 * Helper method that can send data using ChunkedDataSender
	 */
	void send_with_sender(int epol_fd, std::unique_ptr<ChunkedDataSender> chunk_sender);


  public:
	// Constructors
	/**
	 * Constructor that takes the client's socket file descriptor and the
	 * initial state of the client.
	 */
	ConnectedClient(int fd, ClientState initial_state, const SongLibrary* library);

	/**
	 * No argument constructor.
	 */
	ConnectedClient() : client_fd(-1), sender(nullptr), state(RECEIVING), song_library(nullptr) {}


	// Member Functions (i.e. Methods)
	
	/**
	 * Sends the data in the given span to the client.
	 *
	 * @param epoll_fd File descriptor for epoll.
	 * @param data_to_send The data to send.
	 */
	void send_message(int epoll_fd, std::span<char> data_to_send);
	void send_message(int epoll_fd, const std::filesystem::path& file_path);

	/**
	 * Resumes sending data when socket becomes writable again.
	 *
	 * @param epoll_fd File descriptor for epoll.
	 */
	void resume_sending(int epoll_fd);

	/**
	 * Handles new input from the client.
	 *
	 * @param epoll_fd File descriptor for epoll.
	 */
	void handle_input(int epoll_fd);

	/**
	 * Handles a close request from the client.
	 *
	 * @param epoll_fd File descriptor for epoll.
	 */
	void handle_close(int epoll_fd);
};

#endif
