package edu.sandiego.comp375.jukebox;

import java.io.BufferedInputStream;
import java.io.StringBufferInputStream;
import java.net.Socket;
import java.util.Scanner;
import java.util.stream.Stream;

/**
 * Class representing a client to our Jukebox.
 */
public class AudioClient {
	public static void main(String[] args) throws Exception {
		if (args.length != 2) {
			System.out.println("Expected two arguments: server name/ip and port number");
			return;
		}

		int port_num = 0;
		try {
			port_num = Integer.parseInt(args[1]);
		}
		catch (NumberFormatException nfe) {
			System.out.println("Second argument (" + args[1] + ") should be an integer");
			return;
		}

		String server_name = args[0];

		Scanner inputScanner = new Scanner(System.in);
		Thread audioPlayerThread = null;

		while (true) {
			System.out.print(">> ");

			// Get input from user and remove/strip leading and trailing
			// whitespace.
			String command = inputScanner.nextLine().strip();

			if (command.equals("")) {
				// do nothing if they entered nothing
			}
			else if (command.startsWith("play")) {
				try {
					//parse the song ID from comannd such as play 0 ->0
					String[] part = command.split(" ");
					int songID = 0;
					if (part.length>1){
						songID = Integer.parseInt(part[1]);
					}
					Socket socket = new Socket(server_name, port_num);
					if (socket.isConnected()) {
						//send a play request with songID as a 4 byte int
						StreamingMusicProtocol.sendPlayRequest(socket, songID);
						ProtocolHeader responseHeader = StreamingMusicProtocol.receiveHeader(socket);
						//check for an error response
						if (responseHeader.messageType == MessageType.ERROR_RESPONSE){
							String errorMsge = StreamingMusicProtocol.receiveString(socket, responseHeader.payloadLength);
							System.err.println("Error: "+ errorMsge);
							socket.close();
						}
						else{
							System.out.println("Getting response of length: " + responseHeader.payloadLength);
							BufferedInputStream in = new BufferedInputStream(socket.getInputStream(), 2048);
							audioPlayerThread = new Thread(new AudioPlayerThread(in));
							audioPlayerThread.start();
						}
						
						
					}
				}
				catch (NumberFormatException e){
					System.err.println("Error: Invalid song num");
				}
				catch (Exception e) {
					System.out.println(e);
				}
			}
			else if (command.equals("test")){
				try{
					Socket socket = new Socket(server_name, port_num);
					
					if (socket.isConnected()){
						StreamingMusicProtocol.sendHeader(socket, MessageType.LIST_REQUEST, 0);
						//recieving response header
						ProtocolHeader responseHeader = StreamingMusicProtocol.receiveHeader(socket);
						System.out.println("Type of Response: " + responseHeader.messageType);
						System.out.println("Length of Response: "+ responseHeader.payloadLength);
						//reading and print response
						String response = StreamingMusicProtocol.receiveString(socket, responseHeader.payloadLength);
						System.out.println("Response: "+ response);

						socket.close();

					}
				} catch (Exception excep){
					System.out.println(excep);
				}
			}
			else if (command.equals("stop")) {
				if (audioPlayerThread == null) {
					System.err.println("Error: No song is currently playing.");
				}
				else {
					try {
						//send the stop request to the server
						Socket socket = new Socket(server_name, port_num);
						StreamingMusicProtocol.sendHeader(socket, MessageType.STOP_REQUEST, 0);
						socket.close();
					}
					catch (Exception e) {
						System.out.println(e);
					}
					
					//then we stop the audio player thread
					audioPlayerThread.interrupt();
					audioPlayerThread = null;
				}
			}
			else if (command.startsWith("info")) {
				try {
					//parse songID from command
					String[] part = command.split(" ");
					int songID = 0;
					if (part.length > 1){
						songID = Integer.parseInt(part[1]);
					}
					Socket socket = new Socket(server_name, port_num);

					if (socket.isConnected()) {
						//Send info request with songID as 4 byte int
						StreamingMusicProtocol.sendInfoRequest(socket, songID);
					
						//we get back the response
						ProtocolHeader responseHeader = StreamingMusicProtocol.receiveHeader(socket);
						//Check for error response
						if (responseHeader.messageType == MessageType.ERROR_RESPONSE){
							String errorMsge = StreamingMusicProtocol.receiveString(socket, responseHeader.payloadLength);
							System.err.println("Error: "+ errorMsge);
						} 
						else{
							String response = StreamingMusicProtocol.receiveString(socket, responseHeader.payloadLength);
							System.out.println(response);
						}
						socket.close();
					}
				}
					catch (NumberFormatException e){
					System.err.println("Error: Invalid song num");
					}
					catch (Exception e) {
						System.out.println(e);
					}
						
				
			}
			else if (command.equals("list")) {
				try {
					//send the LIST_REQUEST command
					Socket socket = new Socket(server_name, port_num);
					StreamingMusicProtocol.sendHeader(socket, MessageType.LIST_REQUEST, 0);

					//we get back the response
					ProtocolHeader responseHeader = StreamingMusicProtocol.receiveHeader(socket);
					String response = StreamingMusicProtocol.receiveString(socket, responseHeader.payloadLength);

					//print the list of songs
					System.out.println(response);
					socket.close();
				}
				catch (Exception e) {
					System.out.println(e);
				}
			}
			else if (command.equals("exit")) {
				if (audioPlayerThread != null) {
					audioPlayerThread.interrupt();
				}

				System.out.println("Goodbye!");
				break;
			}
			else {
				System.err.println("ERROR: unknown command");
			}
		}

		System.out.println("Client: Exiting");
	}
}
