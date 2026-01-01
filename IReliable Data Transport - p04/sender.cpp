/*
 * File: sender.cpp
 *
 * Simple program that sends data on standard input to a remote host using the
 * RDT library.
 * 
 * You should NOT modify this file.
 */

// C++ standard libraries
#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>

// RDT library
#include "ReliableSocket.h"
#include "logging.h"
#include "aixlog.hpp"

using std::cerr;

int main(int argc, char** argv) {	
	if (argc < 4 || argc > 5) {
		cerr << "Usage: " << argv[0] << " <remote host> <remote port> <file> -v\n";
		exit(1);
	}

	bool verbose_mode = argc == 5;
	initialize_logging(verbose_mode);

	int remote_port_num = std::stoi(argv[2]);

	// Create a reliable connection and connect to the specified remote host
	ReliableSocket socket;
	socket.connect_to_remote(argv[1], remote_port_num);

	std::ifstream input_file;
	input_file.open(argv[3], std::ios::binary);

	// Create a uint8_t array and fill it with 0's
	std::vector<uint8_t> buff(ReliableSocket::MAX_DATA_SIZE);

	auto start_time = std::chrono::system_clock::now();

	auto total_bytes = 0;
	while (!input_file.eof()) {
		input_file.read(reinterpret_cast<char*>(buff.data()), ReliableSocket::MAX_DATA_SIZE);
		auto num_bytes_read = input_file.gcount();
		buff.resize(num_bytes_read);

		total_bytes += num_bytes_read;
		socket.send_data(buff);
		cerr << "sender: sent " << num_bytes_read << " bytes of app data\n";
	}

	auto end_time = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end_time - start_time;

	cerr << "\nFinished sending, closing socket.\n";
	socket.close_connection();

	cerr << "\nSent " << total_bytes << " bytes in " 
			<< elapsed_seconds.count() << " seconds "
			<< "(" << total_bytes / elapsed_seconds.count() << " Bps)\n";

	cerr << "Estimated RTT:  " << socket.get_estimated_rtt() << " ms\n";

	return 0;
}
