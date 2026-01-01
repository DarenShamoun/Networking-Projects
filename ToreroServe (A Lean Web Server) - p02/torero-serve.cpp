/**
 * File: torero-serve.cpp
 *
 * Implementation of ToreroServe webserver.
 *
 * Author 1: Daren Shamoun (darenshamoun@sandiego.edu)
 * Author 2: Phillip Banky (pbanky@sandiego.edu)
 */

// C++ standard libraries
#include <span>
#include <array>
#include <vector>
#include <thread>
#include <string>
#include <iostream>
#include <fstream>
#include <system_error>
#include <filesystem>
#include <regex> // for parsing HTTP requests

// headers for Client and Server socket classes
#include "ClientSocket.hpp"
#include "ServerSocket.hpp"
#include "BoundedBuffer.hpp"

// shorten the std::filesystem namespace down to just fs
namespace fs = std::filesystem;

// shortening std::cout to just cout (and so on)
using std::cout;
using std::string;
using std::array;
using std::vector;
using std::span;
using std::thread;

void sendResponse(ClientSocket client, string resource);
void sendFile(ClientSocket client, string file_path);

/** 
 * Returns the content type for a given file path.
 * Basically, this function looks at the file extension and
 * returns the appropriate MIME type.
 * For Dr. Sat, the fs::path is useful because it has a function
 * to easily get the file extension ... I'm taking path as a param and 
 * returning a string because that's what the rest of the code uses.
 * 
 * @param path The file path.
 * @return The MIME type as a string.
 */
static std::string getPathExtension(const string& path) {
	auto ext = fs::path(path).extension().string();
	if (ext == ".html" || ext == ".htm") return "text/html";
	if (ext == ".css") return "text/css";
	if (ext == ".js") return "application/javascript";
	if (ext == ".png") return "image/png";
	if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
	if (ext == ".gif") return "image/gif";
	if (ext == ".pdf") return "application/pdf";
	if (ext == ".json") return "application/json";
	if (ext == ".txt") return "text/plain";
	return "application/octet-stream"; 
}

/**
 * Generates the requested directories HTML based on the contents of the directory
 * 
 * @param full_file_path The path to the requested resource being sent including the serving directory.
 * @param resource The path to the requested resource without the serving directory
 */
string generateDirectoryHTML(const string& full_file_path, const string& resource) {
	// create the intial HTML with the directory name
	string full_html = "<html>\n<body>\n";
	full_html += "<h1>Contents of " + resource + ":</h1>\n";
	full_html += "<ul id=\"fileList\">\n";

	// loop through the items in the directory and add them to the HTML surrounded by link tags
	for(auto& entry : fs::directory_iterator(full_file_path)) {
		string filename = entry.path().filename().string();
		if(fs::is_directory(entry.path())) {
			full_html += "<li><a href=\"" + filename + "/\">" + filename + "/</a></li>\n";
		} else {
			full_html += "<li><a href=\"" + filename + "\">" + filename + "</a></li>\n";
		}
	}

	// close off the end of the HTML 
	full_html += "</ul>\n";
	full_html += "</body>\n";
	full_html += "</html>\n";

	return full_html;
}

/**
 * Sends a 200 OK response to the client. It will leverage the getPathExtension method
 * made above to determine the content type of the file being sent. Within an OK header typically
 * the content length is also sent, so that is included as a parameter. We also need the file path
 * to determine the content type. and the client to send the header to.
 * 
 * @param client The client to respond to.
 * @param file_path The path to the file being sent.
 */
void sendOKHeader(ClientSocket client, const std::string& file_path) {

	//i need the file size to include in the header
	size_t file_size = fs::file_size(file_path);

	//determine the content type based on the file extension
	string content_type = getPathExtension(file_path);

	//build the header
	string header = 
		"HTTP/1.0 200 OK\r\n"
		"Content-Type: " + content_type + "\r\n"
		"Content-Length: " + std::to_string(file_size) + "\r\n"
		"\r\n";

	client.sendData(header);
}

/**
 * Sends a 200 OK response. containing the header and either the generated HTML or the file requested
 * 
 * @param client The client to respond to.
 * @param resource The path to the requested resource without the serving directory
 * @param full_file_path The path to the requested resource being sent including the serving directory.
 */
void respondWith200(ClientSocket client, string resource, string full_file_path) {
	if(fs::is_directory(full_file_path)) {
		// if the directory has and index.html file display that instead of generated HTML
		if(fs::exists((full_file_path + "/index.html"))) {
			sendOKHeader(client, (full_file_path + "/index.html"));
			sendFile(client, (full_file_path + "/index.html"));
		}
		// if its a directory without and index.html file build the html and send the header and built html
		else {
			string html = generateDirectoryHTML(full_file_path, resource);
			string header = 
				"HTTP/1.0 200 OK\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: " + std::to_string(html.size()) + "\r\n"
				"\r\n";

			client.sendData(header);
			client.sendData(html);
		}
	}
	// if its a regular file send the OK header and the full file
	else if(fs::is_regular_file(full_file_path)) {
		sendOKHeader(client, full_file_path);
		sendFile(client, full_file_path);
	}
}

/**
 * Sends a 400 BAD REQUEST response to the client.
 *
 * @param client The client to respond to.
 */
void respondWith400(ClientSocket client) {
	client.sendData("HTTP/1.0 400 BAD REQUEST\r\n\r\n");
}

/**
 * Sends a 404 NOT FOUND response to the client.
 *
 * @param client The client to respond to.
 */
void respondWith404(ClientSocket client) {
	string response = 
		"<html>\n"
		"<head>\n"
		"<title>Ruh-roh! Page not found!</title>\n"
		"</head>\n"
		"<body>\n"
		"404 Page Not Found! :'( :'( :'(\n"
		"</body>\n"
		"</html>\n";
	
	string response_length = std::to_string(response.size());

	string header = 
		"HTTP/1.0 404 NOT FOUND\r\n"
		"Content-Type: text/html\r\n"
		"Content-Length: " + response_length + "\r\n"
		"\r\n";
	
	client.sendData(header);
	client.sendData(response);
}

/**
 * Parses the given HTTP request message and returns the resource requested.
 * First, we're we get the end of the first line (the request line) by looking
 * for the first occurrence of "\r\n". This is where the request line ends.
 * 
 * Second, we use a regular expression to match the request line against the expected format.
 * Third, if the request line matches the regex, we extract the resource path from the match results.
 *
 * For example, calling this function with the request "GET
 * /apple/varieties.html HTTP/1.1" would return /apple/varieties.html.
 *
 * @param http_request_message
 */
string parseRequest(string http_request_message) {
	// first, read the first line of the http request
	size_t endOfFirstLine = http_request_message.find("\r\n");
	string first_line = (endOfFirstLine == string::npos) ? http_request_message : http_request_message.substr(0, endOfFirstLine);

	// second, use regex to parse the first line
	static const std::regex requestLine_regex(R"(GET\s+([^\s]+)\s+HTTP\/\d\.\d)"); 
	std::smatch results; //STRING MATCHING RESULTS, keeps track of what was matched and allows access to sub-matches when you say results[1], [2], etc
	// if the request doesn't match the regex, it's a bad request
	if(std::regex_match(first_line, results, requestLine_regex) == false) {
		return ""; // bad request
	}

	//third, extract the resource path from the match results
	if(results.size() != 2) { // we expect 2 matches: the whole line and the resource path
		return ""; // bad request
	}

	return results[1]; // return the resource path
}

/**
 * Sends an appropriate HTTP response to the specified client based on the
 * requested resource.
 *
 * @param client The client to respond to
 * @param resource The resource (e.g. "/index.html") requested by the client.
 */
void sendResponse(ClientSocket client, string resource) {
	string full_file_path = "WWW" + resource;
	
	//handle a 400
	if(resource.empty()){ 
		respondWith400(client);
		return;
	}

	//handle a 404
	if(fs::exists(full_file_path) == false){
		respondWith404(client);
		return;
	}

	//handle a 200
	respondWith200(client, resource, full_file_path);
}

/**
 * Sends the file at the given path to the client, sending at most 4096 bytes
 * at a time.
 *
 * @param client The client to send the data to.
 * @param file_path Path to the file (including the file name)
 */
void sendFile(ClientSocket client, string file_path) {
	// open the file in binary mode (works for both text and binary files)
	std::ifstream file(file_path, std::ios::binary);

	// create C++-style array of characters (to store file data)
	array<char, 4096> file_data;

	// keep reading while we haven't reached the end of the file (EOF)
	while (!file.eof()) {
		file.read(file_data.data(), file_data.size()); // read up to 4096 bytes into file_data
		int bytes_read = file.gcount(); // find out how many bytes we actually read

		// send the up to 4096 byte chunk of data we have read  
		if(bytes_read > 0) {
			cout << "Read " << bytes_read << " bytes from file: " << file_path << "\n";
			client.sendData(span(file_data.begin(), bytes_read));
		}
	}
	file.close();
}

/**
 * Receives a request from a connected HTTP client and sends back the
 * appropriate response.
 *
 * @note After this function returns, client will have been closed (i.e.  may
 * not be used again).
 *
 * @param client The client with whom to communicate.
 */
void handleClient(ClientSocket client) {

	// Step 1: Receive the request message from the client
	vector<char> request = client.receiveData(2048);

	// Create a string out of the received data
	string request_string(request.begin(), request.end());
	
	// Step 2: Parse the request string to determine what response to generate.
	string resource = parseRequest(request_string);
	
	// Step 3: Genereate and send an appropriate response to the client
	sendResponse(client, resource);
	
	// Step 4: Close connection with client.
	client.close();
}


/** 
 * Function that continuously consumes clients from the bounded buffer and handles them.
 * Gets an item from a shared bounded buffer that contains clientsockets and then calls handleClient
 * 
 * @param buffer The bounded buffer from which to consume clients.
 */
void consumeClients(BoundedBuffer& buffer) {
	while(true) {
		ClientSocket client = buffer.getItem(); // get a client from the buffer
		handleClient(client); // handle the client
	}
}

/**
 * Runs the webserver on the given port, serving the files in the given
 * directory.
 *
 * @param port The port on which to listen for connections.
 * @param root_dir The directory where the files to serve are located.
 */
void runServer(unsigned short port, string root_dir) {
	cout << "Serving " << root_dir << " on port " << port << std::endl;

	BoundedBuffer clientsBuffer(5); //9, create a bounded buffer i think dr. sat always uses 5 for the buffer size

	//create 4 workers, each running the consumeClients function
	thread t1(consumeClients, std::ref(clientsBuffer));
	thread t2(consumeClients, std::ref(clientsBuffer));
	thread t3(consumeClients, std::ref(clientsBuffer));
	thread t4(consumeClients, std::ref(clientsBuffer));

	/* Create a socket and start listening for new connections on the
	 * specified port. */
	ServerSocket server(port);
	server.startListening();

	/* Now let's start accepting connections. */
	while (true) {
		ClientSocket client = server.acceptConnection();
		clientsBuffer.putItem(client);
	}
}
