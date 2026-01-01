/**
 * protocol.cpp
 *
 * Implementation of protocol-related classes and functions.
 */
#include <arpa/inet.h> // for functions to convert between host and network order (e.g. htonl)

#include "protocol.h"

using std::vector;
using std::span;

template <typename T>
T unpack(span<char> byte_array, size_t offset) {
	T val;
	std::copy_n(byte_array.begin()+offset, sizeof(T), static_cast<char*>(static_cast<void*>(&val)));
	return val;
}

Header::Header(message_type type, uint32_t len) : type(type), len(htonl(len)) {}

Header::Header(span<char> byte_array) : type(unpack<message_type>(byte_array, 0)), len(unpack<uint32_t>(byte_array, 1)) {}

message_type Header::get_type() const { return type; }
uint32_t Header::get_len() const { return ntohl(len); }

vector<char> Header::to_byte_array() {
	vector<char> bytes(Header::size);

	size_t current_loc = 0;

	// copy "type" to beginning of array
	std::copy_n(static_cast<const char*>(static_cast<const void*>(&this->type)),
				sizeof(message_type), bytes.begin()+current_loc);

	// copy "len" after type 
	current_loc += sizeof(message_type); // advance index past type

	std::copy_n(static_cast<const char*>(static_cast<const void*>(&this->len)),
				sizeof(uint32_t), bytes.begin()+current_loc);

	return bytes;
}
