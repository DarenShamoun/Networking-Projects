#ifndef __RELIABLESOCKET_H__
#define __RELIABLESOCKET_H__
/*
 * File: ReliableSocket.h
 *
 * Header / API file for library that provides reliable data transport over an
 * unreliable link.
 *
 */
#include <span>
#include "UnreliableSocket.h"
#include "RDTSegment.h"

enum connection_status { INIT, WAIT_HS2, WAIT_HS3, WAIT_SEND, WAIT_DATA0, WAIT_DATA1, WAIT_ACK0, WAIT_ACK1, CLOSING, CLOSED };

/**
 * Class that represents a socket using a reliable data transport protocol.
 * This socket uses a stop-and-wait protocol so your data is sent at a nice,
 * leisurely pace.
 */
class ReliableSocket {
public:
	/*
	 * You probably shouldn't add any more public members to this class.
	 * Any new functions or fields you need to add should be private.
	 */
	
	// These are constants for all reliable connections
	static const int MAX_SEG_SIZE  = 1400;
	static const int MAX_DATA_SIZE = MAX_SEG_SIZE - sizeof(RDTHeader);

	/**
	 * Basic Constructor, setting estimated RTT to 100 and deviation RTT to 10.
	 */
	ReliableSocket();

	/**
	 * Connects to the specified remote hostname on the given port.
	 *
	 * @param hostname Name of the remote host to connect to.
	 * @param port_num Port number of remote host.
	 */
	void connect_to_remote(char *hostname, int port_num);

	/**
	 * Waits for a connection attempt from a remote host.
	 *
	 * @param port_num The port number to listen on.
	 */
	void accept_connection(int port_num);

	/**
	 * Send data to connected remote host.
	 *
	 * @param buffer The data to send.
	 */
	void send_data(std::span<uint8_t> buffer);

	/**
	 * Receives data from remote host using a reliable connection.
	 *
	 * @param buffer The location where received data will be stored.
	 * @return The amount of data actually received.
	 */
	uint16_t receive_data(std::array<uint8_t, MAX_DATA_SIZE>& buffer);

	/**
	 * Closes an connection.
	 */
	void close_connection();

	/**
	 * Returns the estimated RTT.
	 * 
	 * @return Estimated RTT for this socket (in milliseconds)
	 */
	uint32_t get_estimated_rtt();

private:
	// Private member variables are initialized in the constructor
	UnreliableSocket socket;
	connection_status state;

	uint8_t sequence_number;
	int estimated_rtt;
	int dev_rtt;

	// In the (unlikely?) event you need a new field, add it here.

	/*
	 * Add new member functions (i.e. methods) after this point.
	 * Remember that only the comment and header line goes here. The
	 * implementation should be in the .cpp file.
	 */

	/**
	 * Sends an ACK message with the given ACK number.
	 *
	 * @param ack_number
	 */
	void send_ack(uint8_t ack_number);

	/**
	 * Repeatedly sends the given segment until receiving the expected ACK.
	 *
	 * @note The expected ACK is based on the current state.
	 *
	 * @param segment The segment to send.
	 */
	void send_until_acked(RDTSegment& segment);

	/**
	 * Updates estimated and dev RTT based on the given sample RTT and uses
	 * these values to set the timeout according to the equation timeout =
	 * estimated_rtt + 4*dev_rtt
	 *
	 * @param sample_rtt A new sample RTT to use for calculating estimated
	 * RTT.
	 */
	void update_rtt(uint32_t sample_rtt);
};
#endif
