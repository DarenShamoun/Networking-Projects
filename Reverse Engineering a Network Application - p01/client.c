/**
 * client.c
 *
 * @author Daren Shamoun
 *
 * USD COMP 375: Computer Networks
 * Project 1
 *
 * This is my reconstruction of the sensor network program.
 */

#define _XOPEN_SOURCE 600

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include "io_helpers.h"

/**
 * Type that represents the possible actions that a user could select while running
 * the program.
 */
typedef enum RequestType {
	TEMPERATURE = 1,
	HUMIDITY = 2,
	WIND_SPEED = 3,
	QUIT = 4,
	INVALID
} RequestType;


// forward declarations (yay for archaic C rules)
void main_loop();
RequestType get_user_request();
void handle_request(RequestType request, int server_socket);
int connect_to_weather_station(int server_socket);
void get_sensor_data(int weather_station_fd, RequestType request);
int send_text(int any_fd, const char *str);
int recv_line(int any_fd, char *buffer, size_t buffer_length);
int choose_sensor_command(RequestType request, char **sensor_command, char **sensor_name);


int main() {
	main_loop();
	return 0;
}


/**
 * The main loop get user input and handle selection.
 * until the user quits.
 */
void main_loop() {
	printf("WELCOME TO THE COMP375 SENSOR NETWORK\n\n\n");
	bool done = false;
	while (!done) {
		RequestType selection = get_user_request();
		int server_fd = -1;

		switch (selection) {
			case TEMPERATURE:
				server_fd = connect_to_host("hopper.sandiego.edu", "7030");
				handle_request(selection, server_fd); break;
			case HUMIDITY:
				server_fd = connect_to_host("hopper.sandiego.edu", "7030");
				handle_request(selection, server_fd); break;
			case WIND_SPEED:
				server_fd = connect_to_host("hopper.sandiego.edu", "7030"); 
				handle_request(selection, server_fd); break;
			case QUIT: 
			printf("GOODBYE!\n"); done = true; break;
			default: 
			fprintf(stderr, "\n*** Invalid selection\n\n"); break;
		}		
	}
}


/** 
 * Print command prompt to user and obtain user input.
 *
 * @return The user's desired selection, or -1 if invalid selection.
 * 
 */
RequestType get_user_request() {
	printf("Which sensor would you like to read:\n\n");
	printf("        (1) Air temperature\n");
	printf("        (2) Relative humidity\n");
	printf("        (3) Wind speed\n");
	printf("        (4) Quit Program\n\n");
	printf("Selection: ");

	char input[10];
    get_text_input(input, 10, stdin);

	char *new_line = strchr(input, '\n');
	if (new_line != NULL) new_line[0] = '\0';

	char *end;
	long selection = strtol(input, &end, 10);

	if (end == input || *end != '\0') {return INVALID;}
	else if (selection < 1 || selection > 4) {return INVALID;}
	else {return selection;}
}


/**
 * Talks to the server to get and display the requestied weather information.
 * Handles exactly one request.
 * 
 * @pre Request type is not QUIT or INVALID
 * 
 * @param request The type of info being requested
 * @param server_socket The socket used for communicating with the server
 */
void handle_request(RequestType request, int server_socket) {
	int weather_station_fd = connect_to_weather_station(server_socket);
	get_sensor_data(weather_station_fd, request);
}


/**
 * Authenticate to hopper, parse the CONNECT line, connect+auth to the weather station,
 * and return the weather station socket FD.
 *
 * @param server_socket Connected socket FD for hopper.
 *
 * @return Weather station socket FD on success or -1 on error.
 */
int connect_to_weather_station(int server_socket){
	char buffer[256];
	int num_bytes;
	char weather_station_host[28];
	char weather_station_port[10];
	char weather_station_pass[14];

	if ((send_text(server_socket, "AUTH password123\n")) < 0) return -1;
	if ((num_bytes = recv_line(server_socket, buffer, sizeof buffer)) < 0) return -1; 
	close(server_socket);

	if (sscanf(buffer, "CONNECT %s %s %s", weather_station_host, weather_station_port, weather_station_pass) != 3) 
		{ fprintf(stderr, "error in sscanf"); return -1; }

	int weather_station_fd = connect_to_host(weather_station_host, weather_station_port);

	char weather_station_auth_command[32];
	snprintf(weather_station_auth_command, sizeof weather_station_auth_command, "AUTH %s\n", weather_station_pass);

	if ((send_text(weather_station_fd, weather_station_auth_command)) < 0) return -1;
	if ((num_bytes = recv_line(weather_station_fd, buffer, sizeof buffer)) < 0) return -1;
	return weather_station_fd;
}

/**
 * Send the sensor command, parses the response line, 
 * and prints out the formatted output, then sends 
 * "CLOSE\n" and closes the weather station socket.
 *
 * @param weather_station_fd Connected and authenticated weather station socket.
 * @param request The selected sensor type.
 */
void get_sensor_data(int weather_station_fd, RequestType request){
	char* sensor_command;
	char* sensor_name;
	int num_bytes;
	char buffer[256];
	time_t timestamp;
	int sensor_value;
	char units[10];

	if (choose_sensor_command(request, &sensor_command, &sensor_name) < 0) fprintf(stderr, "Invalid request");
	if ((send_text(weather_station_fd, sensor_command)) < 0) return;
	if ((num_bytes = recv_line(weather_station_fd, buffer, sizeof buffer)) < 0) return;

	sscanf(buffer, "%ld %d %9s", &timestamp, &sensor_value, units);
	if (sscanf(buffer, "%ld %d %9s", &timestamp, &sensor_value, units) != 3) 
		{ fprintf(stderr, "error in sscanf"); close(weather_station_fd); return; }

	char *time_str = ctime(&timestamp);
    time_str[strlen(time_str) - 1] = '\0';

    printf("\nThe last %s reading was %d %s, taken at %s\n\n", sensor_name, sensor_value, units, time_str);

	if ((send_text(weather_station_fd, "CLOSE\n") < 0)) return;

	close(weather_station_fd);
}

/**
 * Helper to send a string over a connected socket using send().
 *
 * @param any_fd Socket FD to send on.
 * @param str    C string to send (without implicit '\0' over the wire).
 *
 * @return Number of bytes sent on success; -1 on error.
 */
int send_text(int any_fd, const char *str){
	int num_bytes = send (any_fd, str, strlen(str), 0);
	if (num_bytes == -1){
		perror("error when sending data");
        close(any_fd);
	}
	return num_bytes;
}


int recv_line(int any_fd, char *buffer, size_t buffer_length){
	if (buffer_length == 0) return -1;
	int num_bytes = recv(any_fd, buffer, buffer_length - 1, 0);
	if (num_bytes == -1){
		perror("error when data");
		close(any_fd);
	}
	buffer[num_bytes] = '\0';
	return num_bytes;
}

/**
 * reads data returning from TCP into buffer  
 *
 * @param any_fd         Socket FD to read from.
 * @param buffer         Buffer to put data into.
 * @param buffer_length  Size of buffer in bytes.
 *
 * @return Number of bytes read or -1 on error.
 */
int choose_sensor_command(RequestType request, char **sensor_command, char **sensor_name){
	switch (request) {
		case TEMPERATURE:
			*sensor_command = "AIR TEMPERATURE\n";
			*sensor_name = "AIR TEMPERATURE";
			return 0;
		case HUMIDITY:
			*sensor_command = "RELATIVE HUMIDITY\n";
			*sensor_name = "RELATIVE HUMIDITY";
			return 0;
		case WIND_SPEED:
			*sensor_command = "WIND SPEED\n";
			*sensor_name = "WIND SPEED";
			return 0;
		default:
			return -1;
	}
}
