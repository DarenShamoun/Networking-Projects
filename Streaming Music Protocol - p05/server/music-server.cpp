/**
 * File: music-server.cpp
 *
 * Authors: Daren Shamoun and Dean Repetti
 *
 * Implementation of a streaming music server.
 */

// C++ standard libraries
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <filesystem>

// POSIX and OS-specific libraries
#include <sys/epoll.h>

#include "networking.h"
#include "ConnectedClient.h"
#include "SongLibrary.h"

namespace fs = std::filesystem;

using std::cout;
using std::cerr;
using std::string;
using std::vector;
using std::map;

const int MAX_EVENTS = 64;

// forward declarations
void setup_new_client(int server_socket, map<int, ConnectedClient>& clients, int epoll_fd, const SongLibrary* library);
void event_loop(int epoll_fd, int server_socket, const SongLibrary* library);
uint32_t find_mp3_files(string dir);

int main(int argc, char **argv) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <port> <filedir>\n";
        exit(EXIT_FAILURE);
    }

	if (!fs::is_directory(argv[2])) {
		cerr << "ERROR: " << argv[2] << " is not a directory\n";
		exit(EXIT_FAILURE);
	}

    // Get the port number from the arguments.
    uint16_t port = static_cast<uint16_t>(std::stoul(argv[1]));

	int server_socket = setup_server_socket(port);

	SongLibrary library;
    library.scan_files(string(argv[2]));
    cout << "Found " << library.num_songs() << " songs.\n";

	// Create the epoll, which returns a file descriptor for us to use later.
	int epoll_fd = epoll_create1(0);
	if (epoll_fd < 0) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	// We want to watch for input events (i.e. connection requests) on our
	// server socket.
	add_socket_to_epoll(server_socket, EPOLLIN, epoll_fd);

    event_loop(epoll_fd, server_socket, &library);
}

/**
 * Accepts a new client then sets the server up to be ready to receive data
 * from that client.
 * After exiting, we'll have a new client set to RECEIVING mode, our socket to
 * that client will be non-blocking, and our epoll interest list will contain
 * this new client (watching for inputs or closes from the client).
 *
 * @param server_socket Socket listening for new connections.
 * @param clients Mapping between client sockets and client info
 * @param epoll_fd File descriptor for epoll
 */
void setup_new_client(int server_socket, map<int, ConnectedClient>& clients, int epoll_fd, const SongLibrary* library) {
	int client_fd = accept_connection(server_socket);
	cout << "Accepted a new connection!\n";

	// Set this to non-blocking mode so we never get hung up
	// trying to send or receive from this client.
	set_non_blocking(client_fd);

	// Watch for "input" and "hangup" events for new clients.
	add_socket_to_epoll(client_fd, EPOLLIN | EPOLLRDHUP, epoll_fd);

	// We have a new client so we'll create a new ConnectClient object to
	// represent this new client.
	ConnectedClient cc(client_fd, RECEIVING, library);

	// add this new connected client to our map from socket fd to connected
	// client.
	clients[client_fd] = std::move(cc);
}

/**
 * Waits for epoll events then handles them accordingly.
 *
 * @param epoll_fd File descriptor for our epoll.
 * @param server_socket Socket that is listening for connections.
 */
void event_loop(int epoll_fd, int server_socket, const SongLibrary* library) {
	// associate client's file descriptor with its ConnectedClient object
	map<int, ConnectedClient> clients;

    while (true) {
		// wait for some events to occur, writing them to our events array
		struct epoll_event events[MAX_EVENTS];

		int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (num_events < 0) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

		// Loop through all the I/O events that just happened.
		for (int n = 0; n < num_events; n++) {
			// Check if this is a "hang up" event (i.e. client closed the
			// connection).
			if ((events[n].events & EPOLLRDHUP) != 0) {
				// If we get here, the socket associated with this event was
				// closed by the remote host so we should clean up.
				clients[events[n].data.fd].handle_close(epoll_fd);
				clients.erase(events[n].data.fd);
			}

			// Check if this is an "input" event (i.e. ready to "read" from
			// this socket)
			else if ((events[n].events & EPOLLIN) != 0) {
				if (events[n].data.fd == server_socket) {
					/*
					 * If the server socket is ready for "reading," that implies
					 * we have a new client that wants to connect so lets
					 * set up that new client now.
					 */
					setup_new_client(server_socket, clients, epoll_fd, library);
				}
				else {
					/*
					 * This wasn't the server socket so this means we have a
					 * client that has sent us data so we can receive it now
					 * without worrying about blocking.
					 */
					clients[events[n].data.fd].handle_input(epoll_fd);
				}
            }

			// Check if this is an "output" event.
			// Note: You may want/need to make this an else if, depending on
			// how you are handling clients.
			if ((events[n].events & EPOLLOUT) != 0) {
				/* 
				 * If you set things up correctly, you should only reach this
				 * point if you started sending a response, but had to stop.
				 * You'll therefore need to continue sending whatever response
				 * you had in progress.
				 */

				/* 
				 * TODO: Create a new function in your ConnectedClient class
				 * and call that here, sort of like what was done for
				 * handle_input and handle_close earlier in this function.
				 */
				clients[events[n].data.fd].resume_sending(epoll_fd);
            }
        }
    }
}

/**
 * Given a path to a directory, this function searches the directory for any
 * files that end in ".mp3".
 * When it files an MP3 file, it also looks for an associated ".info" file, and
 * prints the contents of this file if it exists.
 *
 * @info You'll need to edit this to meet your needs (i.e. don't expect this
 * to do everything you want without any effort). I would recommend making it
 * return a vector (C++'s version of Java's ArrayList) containing the paths to
 * each of the mp3 files you find.
 *
 * @param dir String that represents the path to the directory that you want
 * 				to check.
 *
 * @return Number of MP3 files found inside of the specified directory.
 */
uint32_t find_mp3_files(string dir) {
	uint32_t num_mp3_files = 0;

	// Loop through all files in the directory
	for(fs::directory_iterator entry(dir); entry != fs::directory_iterator(); ++entry) {
		string filename = entry->path().filename().string();

		// See if the current file is an MP3 file
		if (entry->path().extension() == ".mp3") {
			cout << "(" << num_mp3_files << ") " << filename << "\n";
			num_mp3_files++;

			// Look for an associated info file
			fs::path info_file_path = entry->path();
			info_file_path = info_file_path.replace_extension(".mp3.info");
			if (fs::is_regular_file(info_file_path)) {
				// read contents of file into a buffer
				std::ifstream t(info_file_path.string());
				std::stringstream buffer;
				buffer << t.rdbuf();
				cout << "Info:\n" << buffer.str() << "\n";
			}
		}
	}

    return num_mp3_files;
}
