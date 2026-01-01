#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>

#include "io_helpers.h"

void get_text_input(char* buffer, size_t buffer_size, FILE* input_file) {
    memset(buffer, 0, buffer_size); // set all characters in input to '\0' (i.e. nul)
    char *read_str = fgets(buffer, buffer_size, input_file);

    // Check if EOF or an error, exiting the program in both cases.
    if (read_str == NULL) {
        if (feof(input_file)) {
            exit(0);
        }
        else if (ferror(input_file)) {
            perror("fgets");
            exit(1);
        }
    }
}

int connect_to_host(char *hostname, char *port) {
	// Step 1: fill in the address info in preparation for setting 
	//   up the socket
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;  // will point to the results

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_INET;       // Use IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// get ready to connect
	if ((status = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	// Step 2: Make a call to socket
	int fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (fd == -1) {
		perror("socket");
		exit(1);
	}

	// Step 3: connect!
	if (connect(fd, servinfo->ai_addr, servinfo->ai_addrlen) != 0) {
		perror("connect");
		exit(1);
	}

	freeaddrinfo(servinfo); // free's the memory allocated by getaddrinfo

	return fd;
}