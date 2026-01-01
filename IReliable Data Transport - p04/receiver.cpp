/*
 * File: receiver.cpp
 *
 * Simple program that receives data from a remote host using the
 * RDT library, writing the received data to standard output.
 * 
 * You should NOT modify this file.
 */

// C++ standard libraries
#include <string>
#include <chrono>
#include <iostream>
#include <array>
#include <memory>

#include "ReliableSocket.h"
#include "logging.h"
#include "aixlog.hpp"

using std::cerr;

int main(int argc, char **argv) {	
	if (argc < 3 || argc > 4) { 
		cerr << "Usage: " << argv[0] << " <listening port> <file> [-v]\n";
		exit(EXIT_FAILURE);
	}

	auto output_file = fopen(argv[2], "w+");
	if (output_file == NULL) {
		cerr << "ruh roh!\n";
	}

	bool verbose_mode = argc == 4;
	initialize_logging(verbose_mode);

	ReliableSocket socket;
	socket.accept_connection(std::stoi(argv[1]));

	auto start_time = std::chrono::system_clock::now();
	std::array<uint8_t, ReliableSocket::MAX_DATA_SIZE> segment;
	int bytes_received = socket.receive_data(segment);		

	// Keep receiving data until we do a receive that gives us 0 bytes.
	int total_bytes = 0;
	while (bytes_received != 0) {
		LOG(INFO) << "received " << bytes_received << " bytes of app data\n";
		total_bytes += bytes_received;

		// write received data to output file
		fwrite(segment.data(), sizeof(uint8_t), bytes_received, output_file);
		fflush(stdout);
		bytes_received = socket.receive_data(segment);		
	}

	fclose(output_file);

	auto end_time = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end_time - start_time;

	cerr << "\nReceived " << total_bytes << " bytes in " 
			<< elapsed_seconds.count() << " seconds "
			<< "(" << total_bytes / elapsed_seconds.count() << " Bps)\n";

	cerr << "\nFinished receiving file, closing socket.\n";
	socket.close_connection();

	fflush(stdout);
}
