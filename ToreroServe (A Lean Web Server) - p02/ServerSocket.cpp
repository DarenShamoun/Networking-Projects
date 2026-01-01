/**
 * File: ServerSocket.cpp
 *
 * Implementation of ServerSocket class.
 *
 * DO NOT MODIFY THIS FILE IN ANY WAY!
 *
 * Author: Sat Garcia (sat@sandiego.edu)
 */

// operating system specific libraries
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

// C standard library
#include <cstdio>
#include <cstdlib>

// C++ standard library
#include <utility>

// This will limit how many clients can be waiting for a connection.
static const int BACKLOG = 10;

#include "ClientSocket.hpp"
#include "ServerSocket.hpp"

ServerSocket::ServerSocket(unsigned short int port_num) : port_num(port_num) {
    this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socket_fd < 0) {
        perror("Creating socket failed");
        exit(1);
    }

    /* 
     * A server socket is bound to a port, which it will listen on for incoming
     * connections.  By default, when a bound socket is closed, the OS waits a
     * couple of minutes before allowing the port to be re-used.  This is
     * inconvenient when you're developing an application, since it means that
     * you have to wait a minute or two after you run to try things again, so
     * we can disable the wait time by setting a socket option called
     * SO_REUSEADDR, which tells the OS that we want to be able to immediately
     * re-bind to that same port. See the socket(7) man page ("man 7 socket")
     * and setsockopt(2) pages for more details about socket options.
     */
    int reuse_true = 1;

    int retval; // for checking return values

    retval = setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_true,
                        sizeof(reuse_true));

    if (retval < 0) {
        perror("Setting socket option failed");
        exit(1);
    }
};

ServerSocket& ServerSocket::operator=(ServerSocket&& other) {
    std::swap(this->socket_fd, other.socket_fd);
    return *this;
}

ServerSocket::~ServerSocket() { close(this->socket_fd); }

void ServerSocket::startListening() {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port_num);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* 
	 * As its name implies, this system call asks the OS to bind the socket to
     * address and port specified above.
	 */
    int retval = bind(this->socket_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (retval < 0) {
        perror("Error binding to port");
        exit(1);
    }

    /* 
	 * Now that we've bound to an address and port, we tell the OS that we're
     * ready to start listening for client connections. This effectively
	 * activates the server socket. BACKLOG (a global constant defined above)
	 * tells the OS how much space to reserve for incoming connections that have
	 * not yet been accepted.
	 */
    retval = listen(this->socket_fd, BACKLOG);
    if (retval < 0) {
        perror("Error listening for connections");
        exit(1);
    }
}

ClientSocket ServerSocket::acceptConnection() {
	/* 
	 * Another address structure.  This time, the system will automatically
	 * fill it in, when we accept a connection, to tell us where the
	 * connection came from.
	 */
	struct sockaddr_in remote_addr;
	unsigned int socklen = sizeof(remote_addr); 

	/* 
	 * Accept the first waiting connection from the server socket and
	 * populate the address information.  The result (sock) is a socket
	 * descriptor for the conversation with the newly connected client.  If
	 * there are no pending connections in the back log, this function will
	 * block indefinitely while waiting for a client connection to be made.
	 */
	int sock = accept(this->socket_fd, (struct sockaddr*) &remote_addr, &socklen);
	if (sock < 0) {
		perror("Error accepting connection");
		exit(1);
	}

	return ClientSocket(sock);
}
