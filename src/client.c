#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "common.h"

struct sockaddr_in client;
struct sockaddr_in server;
static int port; 
int sockfd;

void usage()
{
    printf("Usage: client -p [port] -s [server address]\n");
    printf("Both port and server arguments are required.\n");
}

int main(int argc, char** argv) 
{
	char* s_port;
	char* s_addr;
	int arg;

	while((arg = getopt(argc, argv, "p:s:")) != -1) 
	{
		switch (arg)
		{
			case 'p': 
				s_port = optarg;
				debug_log(INFO, __FILE__, "Setting port to %s\n", s_port);
				break;
			case 's':
				s_addr = optarg;
				debug_log(INFO, __FILE__, "Setting addr to %s\n", s_addr);
				break;
			default: 
				usage();
                exit(-1);
		}
	}
    if (!s_port) 
    {
        debug_log(FATAL, __FILE__, "No port passed as arg.\n");
        exit(-1);
    }
    if (!(port = atoi(s_port))) 
    {
        debug_log(FATAL, __FILE__, "Unable to parse port as number.\n");
        exit(-1);
    }
	if (!s_addr) 
    {
        debug_log(FATAL, __FILE__, "No server address passed as arg.\n");
        exit(-1);
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(s_addr);

    if (!(sockfd = socket(AF_INET, SOCK_STREAM, 0))) 
    {
        debug_log(FATAL, __FILE__, "Unable to open socket: Exiting with error %s\n", strerror(errno));
        exit(-1);
    }
	debug_log(INFO, __FILE__, "Attempting to connect to server: %s:%s\n", s_addr, s_port);
    if (connect(sockfd, (struct sockaddr*) &server, sizeof(server)))
    {
		debug_log(FATAL, __FILE__, "Unable to connect to server. Exiting with error %s\n", strerror(errno));
        exit(-1);
    }
	else 
	{
		debug_log(INFO, __FILE__, "Successfully connected to server %s with port %d\n", s_addr, port);
	}
	//main loop
	char buf[BUF_SZ];

	for(;;)
	{
		//send packets to server
		//get stdin, check if the length is more than a certain packet size limit, pack it in an array and ship it
		memset(buf, 0, BUF_SZ);
		ssize_t bytes_read = read(STDIN_FILENO, buf, BUF_SZ);
		if (bytes_read > 255)
		{
			//split up the message somehow?
			debug_log(INFO, __FILE__, "Message too long -- discarding\n");
		}
		buf[bytes_read] = '\0';
		sendto(sockfd, (void*) buf, sizeof(buf), 0, (struct sockaddr*) &server, sizeof(server));
	}
}
