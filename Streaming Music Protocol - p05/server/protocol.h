#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
/**
 * protocol.h
 *
 * Header file for protocol-related classes and functions.
 */

#include <span>
#include <vector>

/**
 * Returns a copy of the value starting at offset within the given byte array.
 *
 * @param byte_array
 * @param offset Starting point from begining of byte_array with the value.
 */
template <typename T>
T unpack(std::span<char> byte_array, size_t offset);

/**
 * Represents the type of message in our protocol.
 * @note You won't need to change this in lab, but for project 5 this will
 * need to be updated to match your protocol, assuming you have a type field.
 */
enum message_type : uint8_t {
    LIST_REQUEST = 0x00,
    LIST_RESPONSE = 0x01,
    INFO_REQUEST = 0x02,
    INFO_RESPONSE = 0x03,
    PLAY_REQUEST = 0x04,
    PLAY_RESPONSE = 0x05,
    STOP_REQUEST = 0x06,
    AUDIO_DATA = 0x07,
    STREAM_END = 0x08,
    ERROR_RESPONSE = 0xFF
};

/**
 * Represents the message header in our protocol.
 */
class Header {
  public:
	/**
	 * Constructor that creates a header from the given type and len.
	 *
	 * @note These values are stored in network (not host) order.
	 *
	 * @param type
	 * @param len
	 */
	Header(message_type type, uint32_t len);

	/**
	 * Constructor that creates a header from the given byte array.
	 *
	 * @note It is assumed that the header starts at the beginning of the
	 * array and all values in the array are in network order.
	 */
	Header(std::span<char> byte_array);

	/**
	 * Getters for type and len. Values are returned in host order.
	 */
	message_type get_type() const;
	uint32_t get_len() const;

	static const size_t size = sizeof(uint8_t) + sizeof(uint32_t);

	/**
	 * Returns a byte array (vector) representationg of this header.
	 *
	 * @note All fields in the byte array are given in network order.
	 */
	std::vector<char> to_byte_array();

  private:
	// Private Member Variables
	message_type type;
	uint32_t len;
};

#endif
