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
static struct sockaddr_in server;
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
	while((arg = getopt(argc, argv, "ps:")) != -1) 
	{
		switch (arg)
		{
			case 'p': 
				s_port = optarg;
				break;
			case 's':
				s_addr = optarg;
				break;
			default: 
				usage();
                exit(-1);
		}
	}

    if ((port = atoi(s_port))) 
    {
        //printf("Unable to parse port as number. Exiting with error code %s\n", strerror(errno));
        debug_log(FATAL, __FILE__, "Unable to parse port as number.\n");
        exit(-1);
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(s_addr);
    server.sin_port = htons(port);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))) 
    {
        //printf("Unable to open socket. Exiting with error code %s\n", strerror(errno));
        debug_log(FATAL, __FILE__, "Unable to open socket\n");
        exit(-1);
    }
	debug_log(INFO, __FILE__, "Attempting to connect to server: %s:%s\n", s_addr, s_port);
    if (connect(sockfd, (struct sockaddr*) &server, sizeof(server)))
    {
        //printf("Unable to connect to server. Exiting with error code %s\n", strerror(errno));
		debug_log(FATAL, __FILE__, "Unable to connect to server. Exiting with error code %s\n", strerror(errno));
        exit(-1);
    }

}
