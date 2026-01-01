#ifndef __NETWORKING_H__
#define __NETWORKING_H__
/*
 * File: networking.h
 *
 * Header file for network related classes and functions.
 */

/**
 * Accepts a connection and returns the socket descriptor of the new client
 * that has connected to us.
 *
 * @param server_socket Socket descriptor of the server (that is listening)
 * @return Socket descriptor for newly connected client.
 */
int accept_connection(int server_socket);

/**
 * Creates a socket, sets it to non-blocking, binds it to the given port, then
 * sets it to start listen for incoming connections.
 *
 * @param port_num The port number we will listen on.
 * @return The file descriptor of the newly created/setup server socket.
 */
int setup_server_socket(uint16_t port_num);

/* 
 * Use fcntl (file control) to set the given socket to non-blocking mode.
 * With non-blocking mode set, any time you try to call send or recv that
 * would normally block, it will instead immediately return -1 and set errno
 * to EAGAIN (or EWOULDBLOCK).
 *
 * @param sock The file descriptor for the socket you want to make
 * 				non-blocking.
 */
void set_non_blocking(int sock);

/**
 * Adds the given socket to the given epoll, watching for the given types of
 * events to occur.
 *
 * @param socket_fd File descriptor of the socket to add
 * @param events Type of events (e.g. EPOLLIN) to watch for.
 * @param epoll_fd File descriptor of the epoll object to add to.
 */
void add_socket_to_epoll(int socket_fd, uint32_t events, int epoll_fd);

/**
 * Modifies the given socket in the given epoll, setting it to watch for the
 * given types of events to occur.
 *
 * @note The given socket must have already been added to the epoll.
 *
 * @param socket_fd File descriptor of the socket to modify.
 * @param events Type of events (e.g. EPOLLIN) to watch for now.
 * @param epoll_fd File descriptor of the epoll object to modify
 */
void modify_socket_in_epoll(int socket_fd, uint32_t events, int epoll_fd);

/**
 * Removes the given socket from the given epoll.
 *
 * @note The given socket must have already been added to the epoll.
 *
 * @param socket_fd File descriptor of the socket to remove.
 * @param epoll_fd File descriptor of the epoll object to remove from.
 */
void remove_socket_from_epoll(int socket_fd, int epoll_fd);

#endif
