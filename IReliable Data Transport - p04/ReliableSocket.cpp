/*
 * File: ReliableSocket.cpp
 *
 * Reliable data transport (RDT) library implementation.
 *
 * Author(s): Daren Shamoun, Dean Repetti
 *
 */
#include <iostream>
#include <string>
#include <array>
#include <span>
#include <vector>

#include <arpa/inet.h>

#include "ReliableSocket.h"
#include "UnreliableSocket.h"
#include "RDTSegment.h"
#include "rdt_time.h"
#include "aixlog.hpp"

using std::string;
using std::array;
using std::span;

/*
 * NOTE: Function header comments shouldn't go in this file: they should be put
 * in the ReliableSocket header file.
 */

ReliableSocket::ReliableSocket() {
	this->socket = UnreliableSocket();
	this->state = INIT;
	this->sequence_number = 0;
	this->estimated_rtt = 200;
	this->dev_rtt = 10;
}

void ReliableSocket::accept_connection(int port_num) {
	if (this->state != INIT) {
		LOG(FATAL) << "Socket already in use\n";
		exit(EXIT_FAILURE);
	}

	// listen for a connection on the specfied port
	this->socket.bind(port_num);

	// Wait for a segment to come from a remote host
	array<uint8_t, MAX_SEG_SIZE> segment;
	
	//wait for RDT_CONN from client
	while(true) {
		segment.fill(0);
		
		this->socket.receive(segment);
		RDTSegment seg(segment);
		
		// only move forward if we get a connection request
		if(seg.get_type() == RDT_CONN) {
			break;
		}
		else {
			LOG(WARNING) << "Unexpected segment type, expected RDT_CONN\n";
		}
	}

	// send an ACK to acknowledge the connection request
	this->send_ack(2);

	this->socket.set_timeout_length(this->estimated_rtt + 4 * this->dev_rtt);

	// wait for ACK 3 from the client to confirm the connection
	while(true) {
		array<uint8_t, MAX_SEG_SIZE> response;
		response.fill(0);

		try {
			this->socket.receive(response);
			RDTSegment seg(response);

			if (seg.get_type() == RDT_ACK && seg.get_ack_number() == 3) {
				// handshake complete 
				LOG(INFO) << "Connection established successfully\n";
				break;
			}
			else if (seg.get_type() == RDT_CONN) {
				// ACK 2 might have been lost  so client resent CONN resend ACK 2
				this->send_ack(2);
			}
			else {
				LOG(WARNING) << "Unexpected segment\n";
			}
		}
		catch (SocketTimeout& timeout) {
			// timeout waiting for ACK 3 so we resend ACK 2
			this->send_ack(2);
		}
	}

	// switch to the reciever state, ready to start getting data
	this->state = WAIT_DATA0;
	LOG(INFO) << "Connection successfully established\n";
}

void ReliableSocket::connect_to_remote(char *hostname, int port_num) {
	if (this->state != INIT) {
		LOG(FATAL) << "Socket already in use.\n";
		exit(EXIT_FAILURE);
	}

	// set up UDP socket connection to the host
	this->socket.connect(hostname, port_num);
	
	// Send an RDT_CONN message to remote host to initiate an RDT connection.
	LOG(INFO) << "Starting up the connection... sending the initiated RDT_CONN\n";
	RDTSegment conn_message(0, 0, RDT_CONN);
	this->socket.set_timeout_length(this->estimated_rtt + 4 * this->dev_rtt);
	this->state = WAIT_HS2;

	std::vector<uint8_t> send_buffer(conn_message.get_total_size());
	//more logs add? 
	LOG(INFO) << "Starting three-way handshake with " << hostname << ":" << port_num << "\n";
	conn_message.copy_to(send_buffer);

	// keep sending RDT_CONN until we recieve ACK 2 from server
	while (true){
		this->socket.send(send_buffer.data(), conn_message.get_total_size());
		array<uint8_t, MAX_SEG_SIZE> response;
		response.fill(0);
		try{
			this->socket.receive(response);
			RDTSegment seg(response);

			// server acknowledged with ACK 2
			if (seg.get_type() == RDT_ACK && seg.get_ack_number() == 2){
				break;
			}
		}
		catch(SocketTimeout& timeout){
			// timeout occured resend RDT_CONN
			LOG(INFO) << "Timeout waiting on ACK2, attempting to resend RDT_CONN\n";
		}
	}
	
	// send ACK 3 to complete the connection
	this->state = WAIT_HS3;
	this->send_ack(3);

	// connection established, ready to send data
	this->state = WAIT_SEND;
	LOG(INFO) << "Connection established\n";
}


// You should NOT modify this function in any way.
uint32_t ReliableSocket::get_estimated_rtt() {
	return this->estimated_rtt;
}


void ReliableSocket::update_rtt(uint32_t sample_rtt) {
	// Uses TCP's equations for estimated and
	// dev RTT to calculate the new values for this->estimated_err and
	// this->dev_rtt and then call this->socket's set_timeout_length function.
	const double alpha = 0.125;
	const double beta = 0.25;

	this->estimated_rtt = ((1 - alpha) * this->estimated_rtt) + (alpha * sample_rtt);
	this->dev_rtt = ((1 - beta) * this->dev_rtt) + (beta * abs(sample_rtt - this->estimated_rtt));

	int timeout = this->estimated_rtt + 4 * this->dev_rtt;
	this->socket.set_timeout_length(timeout);
}


void ReliableSocket::send_data(span<uint8_t> data) {
	if (this->state != WAIT_SEND) {
		LOG(WARNING) << "Cannot send: Connection not established.\n";
		return;
	}

	LOG(DEBUG) << "Sending " << data.size() << " bytes\n";

	RDTSegment data_segment(this->sequence_number, 0, RDT_DATA, data);

	if (this->sequence_number == 0) {
		this->state = WAIT_ACK0;
	}
	else {
		this->state = WAIT_ACK1;
	}

	this->send_until_acked(data_segment);

	// update sequence number (flip between 0 and 1)
	this->sequence_number = 1 - this->sequence_number;

	this->state = WAIT_SEND;
}


uint16_t ReliableSocket::receive_data(array<uint8_t, MAX_DATA_SIZE>& buffer) {
	if (this->state != WAIT_DATA0 && this->state != WAIT_DATA1) {
		LOG(WARNING) << "Cannot receive: Connection not established.\n";
		return 0;
	}

	// determine which sequence number we expect based on the current state
	uint8_t expected_seq;
	if (this->state == WAIT_DATA0) {
		expected_seq = 0;
	}
	else {
		expected_seq = 1;
	}

	array<uint8_t, MAX_SEG_SIZE> received_segment;
	received_segment.fill(0);

	// set timeoutlength to 0 to indicate that we're willing to wait as long
	// as it takes to get new data from the sender
	this->socket.set_timeout_length(0);

	// wait for data or close request
	while (true) {
		try {
			int count = this->socket.receive(received_segment);
			LOG(DEBUG) << "Received " << count << " bytes of data.\n";
		}
		catch (ConnectionClosed& cc) {
			LOG(FATAL) << "Connection closed unexpectedly.\n";
			exit(EXIT_FAILURE);
		}

		RDTSegment data_segment(received_segment);
		LOG(INFO) << "Received segment: " << data_segment.to_string() << "\n";

		if (data_segment.get_type() == RDT_DATA) {
			if (data_segment.get_sequence_number() == expected_seq) {
				// correct so we ACK this segment and switch state
				this->send_ack(expected_seq);

				// alternate between waiting for 0 and 1 
				this->state = this->state == WAIT_DATA0 ? WAIT_DATA1 : WAIT_DATA0;

				// copy payload to buffer and return size
				data_segment.copy_payload_to(span<uint8_t>(buffer));
				return data_segment.get_payload_size();
			}
			else {
				//wrong sequence number due to old or duplicated segment so we resend last ACK
				uint8_t last_seq = 1 - expected_seq;
				this->send_ack(last_seq);
			}
		}
		else if (data_segment.get_type() == RDT_CLOSE) {
			//sender wants to close the connection
			LOG(DEBUG) << "Received close request.\n";
			this->send_ack(4);
			this->state = CLOSING;
			return 0;
		}
	}
}


void ReliableSocket::close_connection() {
	if (this->state == WAIT_SEND) {
		// This is the sender closing the connection.
		LOG(DEBUG) << "Sender closing its connection.\n";
		
		// Construct an RDT_CLOSE message to indicate to the remote host that we
		// want to end this connection.
		RDTSegment close_message(4, 0, RDT_CLOSE);

		//send the close message until we get back ACK 4
		this->send_until_acked(close_message);
		this->state = CLOSED;
	}

	else if (this->state == CLOSING) {
		// This is the receiver closing the connection.
		LOG(DEBUG) << "Receiving closing its connection.\n";

		// wait for 2 seconds for any duplicate close requests
		this->socket.set_timeout_length(2000);

		array<uint8_t, MAX_SEG_SIZE> received_segment;
		bool waiting = false;

		while(!waiting) {
			try {
				received_segment.fill(0);
				this->socket.receive(received_segment);
				RDTSegment seg(received_segment);

				if(seg.get_type() == RDT_CLOSE) {
					// out ACK might have been lost resend it
					this->send_ack(4);
				}
			}
			catch (SocketTimeout& st){
				//timeout with no more close request safe to close
        		waiting = true;
    		}
			catch (ConnectionClosed& cc) {
				// connection already closed by other end
				waiting = true;
			}
		}	
		this->state = CLOSED;
	}
	this->socket.close();
}

// Do NOT modify this function in any way.
void ReliableSocket::send_ack(uint8_t ack_number) {
	LOG(DEBUG) << "sending ACK(" << +ack_number << ")\n";

	RDTSegment ack_message(0, ack_number, RDT_ACK);

	std::vector<uint8_t> send_buffer(ack_message.get_total_size());
	ack_message.copy_to(send_buffer);
	this->socket.send(send_buffer.data(), ack_message.get_total_size());
}


void ReliableSocket::send_until_acked(RDTSegment& segment) {
	LOG(DEBUG) << "Sending: " << segment.to_string() << "\n";
	std::vector<uint8_t> send_buffer(segment.get_total_size());
	segment.copy_to(send_buffer);

	// determine the expected ACK number based on the segments type
	uint8_t expect_ack;
	if (segment.get_type() == RDT_DATA){
		// for data segments the ACK should match the sequence number
		expect_ack = segment.get_sequence_number();
	} else if (segment.get_type() == RDT_CLOSE){
		// close messages should have ACK 4
		expect_ack = 4;
	} else { 
		this->socket.send(send_buffer.data(), segment.get_total_size());
		return;
	}
	
	// this loop keeps sending until we get the correct ACK back
	while (true){
		int time_sent = current_msec();
		LOG(DEBUG) << "Sending segment at time: " << time_sent << "\n";
		this->socket.send(send_buffer.data(), segment.get_total_size());

		array<uint8_t, MAX_SEG_SIZE> received_data;
		received_data.fill(0); // initialize all bytes in received_data to 0
		try {
			this->socket.receive(received_data);
			int time_recieved = current_msec();
			uint32_t rtt_sample = time_recieved - time_sent;

			RDTSegment response(received_data);
			
			LOG(INFO) << "Received segment: " << response.to_string() << "\n";

			// check if we received the ACK we expected
			if (response.get_type() == RDT_ACK && response.get_ack_number() == expect_ack){
				
				if (segment.get_type() == RDT_DATA){
					this->update_rtt(rtt_sample);
				}
				return;
			}
			// if wrong ACK, loop continues and resends the ACK
		}
		catch (SocketTimeout& st) {
			// timeout happens so resend the ACK
			LOG(INFO) << "Timed out while waiting for response. Resending data.\n";
		}
		catch (ConnectionClosed& cc) {
			LOG(FATAL) << "Connection shutdown by peer.\n";
			exit(EXIT_FAILURE); //another catch?
		}	
	}
}
