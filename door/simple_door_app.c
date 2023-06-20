#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#include "simple_door.h"

#define BOT_TOKEN "6146848351:AAGybcjX4-zldoDGivbF3t_N6cQCWZfnFg4"
#define CHAT_ID 5139327878
#define PORT 443
#define SERVER_IP "https://api.telegram.org/bot6146848351:AAGybcjX4-zldoDGivbF3t_N6cQCWZfnFg4/sendMessage"

int door_fd, socket_fd;
struct hostent *server;
struct sockaddr_in serv_addr;
char message[1024], response[4096];
ssize_t bytes_received;

time_t timer;
struct tm *t;

int construct_msg(char* msg) {
	memset(message, 0, sizeof(message));

	if(strlen(msg) > 30) {
		perror("message is too long (over 30B)\n");
		return -1;
	}

	char my_msg[31] = {0, };
	
	sprintf(my_msg, "chat_id=5139327878&text=%s", msg);

	sprintf(message, 
			"POST / HTTP/1.1\r\n"
			"Host: %s\r\n"
			"Content-Length: %zu\r\n"
			"Content-Type: application/x-www-form-urlencoded\r\n"
			"\r\n"
			"%s",
			SERVER_IP, strlen(my_msg), my_msg);
	return 0;
}

void print_current_time() {
	t = localtime(&timer);
	char msg[100] = {0, };

	sprintf(msg, "Door opened at [%04d/%02d/%02d] %02d:%02d\n",
			t->tm_year + 1900,
			t->tm_mon + 1,
			t->tm_mday,
			t->tm_hour,
			t->tm_min);
	printf("%s", msg);
}

int main(int argc, char *argv[]) {
	
	if((door_fd = open("/dev/simple_door_dev", O_RDWR)) < 0) {
		perror("smart door dev open error\n");
		return -1;
	}

	/*
	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket open error\n");
		return -1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, SERVER_IP, &(serv_addr.sin_addr)) <= 0) {
		perror("Invalid address / address not supported error\n");
		return -1;
	}

	if(connect(socket_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) {
		perror("connection fail error\n");
		return -1;
	}
	
	construct_msg("Door opened!");

	*/

	timer = time(NULL);

	/* Wait for the door to be opened */
	while(1) {
		ioctl(door_fd, WAIT_OPEN, NULL);
		/*

		// send the message to telegram when the door is opened
		memset(response, 0, sizeof(response));

		if(send(socket_fd, message, strlen(message), 0) < 0) {
			perror("message send error!\n");
			return -1;
		}

		if((bytes_received = recv(socket_fd, response, sizeof(response) - 1, 0)) < 0) {
			perror("message receive error!\n");
			return -1;
		}

		response[bytes_received] = '\0';
		printf("=======Response========\n");
		printf("%s\n\n", response); */
		print_current_time();
	}

	close(door_fd);
	//close(socket_fd);
	return 0;
}
