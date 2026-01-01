/**
 * ToreroServe: A Lean Web Server
 * COMP 375 - Project 02
 *
 * This program take two command line parameters:
 * 	1. The port number on which to bind and listen for connections
 * 	2. The directory out of which to serve files.
 *
 * 	DO NOT MODIFY THIS FILE IN ANY WAY!
 */

// C++ standard libraries
#include <string>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <filesystem>

// shortening std::cout to just cout (and so on)
using std::cerr;
using std::string;

namespace fs = std::filesystem;

void runServer(unsigned short int port, string root_dir); // definition located in torero-server.cpp

int main(int argc, char** argv) {
	/* Make sure the user called our program correctly. */
	if (argc != 3) {
		cerr << "Usage: " << argv[0] << " <port> <root dir>\n";
		exit(1);
	}

    // Read the port number from the first command line argument.
    unsigned short int port = 0;

	try {
		port = std::stoi(argv[1]);
	}
	catch (std::invalid_argument const& ex) {
		cerr << "ERROR: " << argv[1] << " is not a valid port number\n";
		exit(1);
	}

	// Confirm that user gave a valid directory for the root
	if (!fs::is_directory(argv[2])) {
		cerr << "ERROR: " << argv[2] << " does not exist or is not a directory\n";
		exit(1);
	}

	runServer(port, string(argv[2]));

	return 0;
}
