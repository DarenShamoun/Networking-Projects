/*
 * File: networking.cpp
 *
 * Implementation file for network related classes and functions.
 */

#include <iostream>

// C standard libraries
#include <cerrno>

// POSIX and OS-specific libraries
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "networking.h"

using std::cout;
using std::cerr;

const int BACKLOG = 10;

int setup_server_socket(uint16_t port_num) {
    /* Create the socket that we'll listen on. */
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    /* Set SO_REUSEADDR so that we don't waste time in TIME_WAIT. */
    int val = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, 
							&val, sizeof(val));
    if (val < 0) {
        perror("Setting socket option failed");
        exit(EXIT_FAILURE);
    }

    /* 
	 * Set our server socket to non-blocking mode.  This way, if we
     * accidentally accept() when we shouldn't have, we won't block
     * indefinitely.
	 */
    set_non_blocking(sock_fd);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_num);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* Bind our socket and start listening for connections. */
    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Error binding to port");
        exit(EXIT_FAILURE);
    }

    if (listen(sock_fd, BACKLOG) < 0) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

	return sock_fd;
}

/**
 * Accepts a connection and returns the socket descriptor of the new client
 * that has connected to us.
 *
 * @param server_socket Socket descriptor of the server (that is listening)
 * @return Socket descriptor for newly connected client.
 */
int accept_connection(int server_socket) {
	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof(their_addr);
	int new_fd = accept(server_socket, (struct sockaddr *)&their_addr,
						&addr_size);
	if (new_fd < 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	return new_fd;
}

/* 
 * Use fcntl (file control) to set the given socket to non-blocking mode.
 * With non-blocking mode set, any time you try to call send or recv that
 * would normally block, it will instead immediately return -1 and set errno
 * to EAGAIN (or EWOULDBLOCK).
 *
 * @param sock The file descriptor for the socket you want to make
 * 				non-blocking.
 */
void set_non_blocking(int sock) {
    // Get the current flags
    int socket_flags = fcntl(sock, F_GETFL);
    if (socket_flags < 0) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

	// Add in the nonblock option
    socket_flags = socket_flags | O_NONBLOCK;

    // Set the new flags, including O_NONBLOCK.
    int result = fcntl(sock, F_SETFL, socket_flags);
    if (result < 0) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }
}

void add_socket_to_epoll(int socket_fd, uint32_t events, int epoll_fd) {
	struct epoll_event event;
	event.data.fd = socket_fd;
	event.events = events;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
		perror("add_socket_to_epoll");
		exit(EXIT_FAILURE);
	}
}

void modify_socket_in_epoll(int socket_fd, uint32_t events, int epoll_fd) {
	struct epoll_event event;
	event.data.fd = socket_fd;
	event.events = events;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socket_fd, &event) == -1) {
        perror("modify_socket_in_epoll");
        exit(EXIT_FAILURE);
    }
}

void remove_socket_from_epoll(int socket_fd, int epoll_fd) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, NULL) == -1) {
        perror("remove_socket_from_epoll");
        exit(EXIT_FAILURE);
    }
}
