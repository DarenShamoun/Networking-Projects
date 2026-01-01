#ifndef __RDTSEGMENT_H__
#define __RDTSEGMENT_H__
/*
 * File: RDTSegment.h
 *
 * Header file for class representing a segment in our Reliable Data Transport
 * (RDT) protocol.
 */

#include <span>

enum RDTMessageType : uint8_t {RDT_DATA, RDT_ACK, RDT_CONN, RDT_CLOSE};

/**
 * Format for the header of a segment send by our reliable socket.
 */
struct RDTHeader {
	uint8_t sequence_number;
	uint8_t ack_number;
	RDTMessageType type;
	uint16_t payload_size;
};

std::string message_type_str(RDTMessageType type);

/**
 * Class representing a segment of the RDT protocol.
 */
class RDTSegment {
public:
	static const uint16_t MAX_SEG_SIZE = 1400;

	// Constructors
	RDTSegment(uint8_t sequence_number, uint8_t ack_number, RDTMessageType type);
	RDTSegment(uint8_t sequence_number, uint8_t ack_number, RDTMessageType type, std::span<uint8_t> payload);
	RDTSegment(const std::span<uint8_t> source);

	/*
	 * Copies this segment's header and payload to the destination span.
	 *
	 * @param destination The location where to store data.
	 */
	void copy_to(std::span<uint8_t> destination);

	/*
	 * Copies this segment's payload to the destination span.
	 *
	 * @param destination The location where to store data.
	 */
	void copy_payload_to(std::span<uint8_t> destination);

	// Getters
	uint8_t get_sequence_number();
	uint8_t get_ack_number();
	RDTMessageType get_type();
	uint16_t get_total_size();
	uint16_t get_payload_size();

	/**
	 * Returns a string representation of this segment.
	 */
	std::string to_string();

private:
	RDTHeader header;
	std::span<uint8_t> payload;
};
#endif
