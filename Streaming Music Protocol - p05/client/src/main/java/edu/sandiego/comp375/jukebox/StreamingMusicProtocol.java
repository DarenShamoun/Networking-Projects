/**
 * This file contains a set of classes / static methods to aid in sending and
 * receiving a streaming music protocol messages.
 *
 */
package edu.sandiego.comp375.jukebox;

import java.net.*;
import java.io.*;
import java.nio.*;
import java.nio.charset.StandardCharsets;

/**
 * Class with static methods meant to aid sending and receiving data over a
 * socket.
 */
public class StreamingMusicProtocol {
	/**
	 * Receives a message header over the given socket.
	 *
	 * @param s The Socket to read data from
	 * @return The header that was read from the socket.
	 */
	public static ProtocolHeader receiveHeader(Socket socket) throws IOException {
		DataInputStream in = new DataInputStream(socket.getInputStream());

		int type = (int)in.readByte();
		int length = in.readInt();

		return new ProtocolHeader(type, length);
	}

	/**
	 * Reads a UTF-8 formatted string from the socket.
	 *
	 * @param socket The Socket to read data from
	 * @param length The expected length of the string
	 * @return The string that was read in from the socket
	 */
	public static String receiveString(Socket socket, int length) throws IOException {
		DataInputStream in = new DataInputStream(socket.getInputStream());

		byte[] message = new byte[length];
		in.readFully(message);
		return new String(message, StandardCharsets.UTF_8);
	}

	/**
	 * Writes a message header to the given stream.
	 *
	 * @param outputStream The stream to write the header to.
	 * @param header The protocol header to write.
	 */
	private static void writeHeader(OutputStream outputStream, ProtocolHeader header) throws IOException {
		ByteBuffer buffer = header.toByteBuffer();
		outputStream.write(buffer.array());
	}


	/**
	 * Writes a 4-byte int to the given stream, using network order (i.e. Big Endian)
	 *
	 * @param outputStream The socket to write to.
	 * @param n The 4-byte integer value to write
	 */
	private static void writeInt(OutputStream outputStream, int n) throws IOException {
		DataOutputStream out = new DataOutputStream(outputStream);
		out.writeInt(n);
	}

	/**
	 * Writes a 2-byte int to the given stream, using network order (i.e. Big Endian)
	 *
	 * @param outputStream The socket to write to.
	 * @param n The 2-byte integer value to write
	 */
	private static void sendShortInt(OutputStream outputStream, short n) throws IOException {
		ByteBuffer buffer = ByteBuffer.allocate(Short.BYTES);
		buffer.order(ByteOrder.BIG_ENDIAN);
		outputStream.write(buffer.array());
	}

	/**
	 * Writes a UTF-8 encoded string to the given stream.
	 *
	 * @param outputStream The socket to write to.
	 * @param data The string data to write.
	 */
	private static void writeString(OutputStream outputStream, String message) throws IOException {
		DataOutputStream out = new DataOutputStream(outputStream);
		byte[] utf8_message = message.getBytes(StandardCharsets.UTF_8);
		out.write(utf8_message);
	}

	/**
	 * Sends the specified message over the given socket, setting the message
	 * type to REQUEST.
	 *
	 * @param socket The socket to send over
	 * @param type The type of message
	 * @param message The message to send over the socket.
	 */
	public static void sendStringMessage(Socket socket, MessageType type, String message) throws IOException {
		// Use a buffered output so we don't send the data until we have it
		// all ready to go.
		BufferedOutputStream outStream = new BufferedOutputStream(socket.getOutputStream());

		// create and send header
		ProtocolHeader header = new ProtocolHeader(type, message.length());
		writeHeader(outStream, header);

		writeString(outStream, message);

		// flush the stream so that the complete message gets sent now
		outStream.flush();
	}

	/**
	 * Sends a protocol header over the given socket.
	 *
	 * @param socket The socket to send over
	 * @param type The type of message
	 * @param message The message to send over the socket.
	 */
	public static void sendHeader(Socket socket, MessageType type, int length) throws IOException {
		// Use a buffered output so we don't send the data until we have it
		// all ready to go.
		BufferedOutputStream outStream = new BufferedOutputStream(socket.getOutputStream());

		// create and send header
		ProtocolHeader header = new ProtocolHeader(type, length);
		writeHeader(outStream, header);

		// flush the stream so that the complete message gets sent now
		outStream.flush();
	}

	public static void main(String[] args) throws IOException {
		// Connect to the server (localhost is the current computer)
		//Socket serverSocket = new Socket("localhost", 5100);
		Socket serverSocket = new Socket("ubuntu22.04.orb.local", 5100);

		sendStringMessage(serverSocket, MessageType.LIST_REQUEST, "bonjour");

		ProtocolHeader h = receiveHeader(serverSocket);
		String responseMessage = receiveString(serverSocket, h.payloadLength);
		System.out.println("response: " + responseMessage);
		serverSocket.close();
	}
	/**
	 * Send a play request with songID as a 4 byte int payload
	 */
	public static void sendPlayRequest(Socket socket, int songID) throws IOException{

		BufferedOutputStream outStream = new BufferedOutputStream(socket.getOutputStream());
		ProtocolHeader header = new ProtocolHeader(MessageType.PLAY_REQUEST, 4);
		writeHeader(outStream, header);
		writeInt(outStream, songID);
		outStream.flush();
	}
	/**
	 * Send an info request with songID as a 4 byte int payload
	 */
	public static void sendInfoRequest(Socket socket, int songID) throws IOException{
		
		BufferedOutputStream outStream = new BufferedOutputStream(socket.getOutputStream());
		ProtocolHeader header = new ProtocolHeader(MessageType.INFO_REQUEST, 4);
		writeHeader(outStream, header);
		writeInt(outStream, songID);
		outStream.flush();
	}
}

/**
 * Class to represent the header of our streaming music protocol.
 */
class ProtocolHeader {
	final MessageType messageType;
	final int payloadLength;

	public ProtocolHeader(MessageType type, int length) {
		this.messageType = type;
		this.payloadLength = length;
	}

	public ProtocolHeader(int type, int length) {
		this.messageType = MessageType.get(type);
		this.payloadLength = length;
	}

	/**
	 * Gets a network order (Big Endian) ByteBuffer filled with the contents
	 * of this header.
	 */
	public ByteBuffer toByteBuffer() {
		// Allocate 5 bytes for the header (1 for type, 4 for length)
		ByteBuffer header = ByteBuffer.allocate(5);

		// Set the order to BIG_ENDIAN, which is what is the standard used for
		// communication of computer networks.
		header.order(ByteOrder.BIG_ENDIAN);

		// Add the message type and length to the buffer
		
		// @note ordinal gets the number associated with the MessageType
		header.put((byte)this.messageType.ordinal());
		header.putInt(this.payloadLength);

		return header;
	}
}

/**
 * Enum representing the type of message, either REQUEST or RESPONSE.
 *
 * INVALID type is here to represent an incorrect message type.
 */
enum MessageType {
    LIST_REQUEST,
    LIST_RESPONSE,
    INFO_REQUEST,
    INFO_RESPONSE,
    PLAY_REQUEST,
    PLAY_RESPONSE,
    STOP_REQUEST,
    AUDIO_DATA,
    STREAM_END,
	REQUEST,  // put this here so old audioserver compiles 
	RESPONSE, // put this here so old audioserver compiles 
    ERROR_RESPONSE,
	INVALID;

	private static final MessageType values[] = values();

	/**
	 * Returns a MessageType based on the given integer. Returns INVALID type
	 * if the integer isn't valid.
	 */
	public static MessageType get(int ordinal) { 
		if (ordinal == 0xFF) {
			return ERROR_RESPONSE;
		}
		if (ordinal < 0 || ordinal >= values.length -1) {
			return INVALID;
		}
		return values[ordinal];
	}
}
