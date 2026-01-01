/*
 * File: RDTSegment.cpp
 *
 * Implementation file for class representing a segment in our Reliable Data Transport
 * (RDT) protocol.
 */
#include <iostream>
#include <string>
#include <span>
#include <arpa/inet.h>
#include <cstring>

#include "RDTSegment.h"
#include "aixlog.hpp"

using std::string;
using std::span;

string message_type_to_string(RDTMessageType type) {
	if (type == RDT_CONN) return "RDT_CONN";
	else if (type == RDT_ACK) return "RDT_ACK";
	else if (type == RDT_DATA) return "RDT_DATA";
	else if (type == RDT_CLOSE) return "RDT_CLOSE";
	else {
		LOG(FATAL) << "Unknown message type\n";
		exit(EXIT_FAILURE);
	}
}

string RDTSegment::to_string() {
	string s = "{";
	s += message_type_to_string(this->header.type);
	s += ": seq=";
	s += std::to_string(this->header.sequence_number);
	s += ", ack=";
	s += std::to_string(this->header.ack_number);
	s += ", payload=";
	s += std::to_string(ntohs(this->header.payload_size));
	s += " bytes}";
	return s;
}

RDTSegment::RDTSegment(uint8_t sequence_number, uint8_t ack_number, RDTMessageType type) {
	this->header.sequence_number = sequence_number;
	this->header.ack_number = ack_number;
	this->header.type = type;
	this->header.payload_size = htons(0);
}

RDTSegment::RDTSegment(uint8_t sequence_number, uint8_t ack_number, RDTMessageType type, span<uint8_t> payload) {
	this->header.sequence_number = sequence_number;
	this->header.ack_number = ack_number;
	this->header.type = type;
	this->header.payload_size = htons(payload.size_bytes());
	this->payload = payload;
}

RDTSegment::RDTSegment(span<uint8_t> source) {
	void *header = &(this->header);
	size_t header_size = sizeof(RDTHeader);

	memcpy(header, source.data(), header_size); // copy header
	this->payload = span<uint8_t>(source.data()+header_size, ntohs(this->header.payload_size));
}

void RDTSegment::copy_to(span<uint8_t> destination) {
	void *header = &(this->header);
	size_t header_size = sizeof(RDTHeader);

	memcpy(destination.data(), header, header_size); // copy header
	std::copy(this->payload.begin(), this->payload.end(), destination.data()+header_size);
}

void RDTSegment::copy_payload_to(span<uint8_t> destination) {
	std::copy(this->payload.begin(), this->payload.end(), destination.data());
}

uint8_t RDTSegment::get_sequence_number() {
	return this->header.sequence_number;
}

uint8_t RDTSegment::get_ack_number() {
	return this->header.ack_number;
}

RDTMessageType RDTSegment::get_type() {
	return this->header.type;
}

uint16_t RDTSegment::get_total_size() {
	return sizeof(RDTHeader) + this->payload.size_bytes();
}

uint16_t RDTSegment::get_payload_size() {
	return this->payload.size_bytes();
}
