#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>
#include "server.h"
static int port;

int main(int argc, char** argv)
{
	char* s_port;
	struct sockaddr_in client_addr;
	int sockfd;
	int connfds[MAXCONN];
	int connection_count = 0;
	int arg;

	while((arg = getopt(argc, argv, "p:")) != -1)
	{
		switch (arg)
		{
			case 'p':
				s_port = optarg;
				break;
			default:
				usage();
				break;
		}
	}
    if (!s_port) 
    {
        debug_log(FATAL, __FILE__, "No port provided.\n");
        usage();
    }
	debug_log(INFO, __FILE__, "using port %s\n", s_port);
	if (!(port = atoi(s_port))) 
    {
        debug_log(FATAL, __FILE__, "Unable to parse port into number.\n");
        usage();
    }
	//bind to a socket and listen to it for incoming connections
	sockfd = listen_server(port);
	int client_addr_len = sizeof(client_addr);
	connfds[connection_count] = accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_len);
	handle_connection(connfds[connection_count]);
	++connection_count;

	//close the socket file descriptor
	close(sockfd);

}

void usage()
{
	printf("Usage: server -p [PORT]\n");
	printf("Begin a TCP server listening on the given port.\n");
    exit(-1);
}

int listen_server(int port)
{
	struct sockaddr_in sa;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (!sockfd)
	{
		debug_log(FATAL, __FILE__, "Failed to open TCP server socket");
		exit(-1);
	}
	debug_log(INFO, __FILE__, "TCP server socket opened on %d\n", sockfd);
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(port);

	if ((bind(sockfd, (struct sockaddr*)&sa, sizeof(sa))) != 0)
	{
		debug_log(FATAL, __FILE__, "Server failed to bind to socket %d\n", sockfd);
		exit(-1);
	}
	if (listen(sockfd, MAXCONN) != 0)
	{
		debug_log(FATAL, __FILE__, "Server failed to listen on socket %d\n", sockfd);
		exit(-1);
	}
	debug_log(INFO, __FILE__, "Server listening on port %d\n", port);
	return sockfd;
}
void handle_connection(int connfd)
{
	char buff[BUFF_SZ];
	ssize_t rcvd;
	for (;;)
	{
		memset(buff, 0, BUFF_SZ);

		rcvd = read(connfd, buff, BUFF_SZ);
		debug_log(INFO, __FILE__, "Read %zd bytes from client with message %s\n", rcvd, buff);
		memset(buff, 0, BUFF_SZ);
		if (strncmp("exit", buff, 4) == 0)
		{
			debug_log(WARN, __FILE__, "Disconnecting client from server\n");
			break;
		}
	}
}
