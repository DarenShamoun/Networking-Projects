#ifndef IO_HELPERS_H
#define IO_HELPERS_H

#include <stdio.h>

/**
 * Reads input into the specified buffer, doing error checking alone the way.
 * 
 * @param buffer The place where input will be stores
 * @param buffer_size The size (in bytes) of the buffer
 * @param input_file The input source
 */
void get_text_input(char* buffer, size_t buffer_size, FILE* input_file);

/**
 * Creates a TCP socket and connects to the host with the given name, using the given port.
 *
 * @param hostname The name of the host to connect to (e.g. "foo.sandiego.edu")
 * @param port The port number to connect to
 * @return File descriptor for the newly created socked.
 */
int connect_to_host(char *hostname, char *port);

#endif