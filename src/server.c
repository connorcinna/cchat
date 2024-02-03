#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include "server.h"
#include "common.h"

//the server port
static uint32_t port; 
//all of the file descriptors open by the server
static uint32_t connfds[MAX_CONN];
//all of the clients the server knows about
static struct sockaddr_in clients[MAX_CONN];
//the total number of connections right now
static uint32_t num_conn = 0;

int32_t main(uint32_t argc, char** argv)
{
	char* s_port;
	uint32_t sockfd;

	uint32_t arg;

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
	while (num_conn < MAX_CONN) 
	{
		struct sockaddr_in client;
		socklen_t client_len = sizeof(client);
		if (!(connfds[num_conn] = accept(sockfd, (struct sockaddr*) &client, &client_len))) 
		{
			debug_log(WARN, __FILE__, "Failed to accept connection from client %d: %s", num_conn, strerror(errno));
		}
		debug_log(INFO, __FILE__, "New client connected on fd: %d\n", connfds[num_conn]);
		pthread_t t;
		if ((pthread_create(&t, NULL, (void*) work, (void*) &connfds[num_conn]))) 
		{
			debug_log(WARN, __FILE__, "Failed to spawn delegate thread\n");
		}
		++num_conn;
	}

	for (;;) 
	{
		if (getchar() == 'q')
		{
			close(sockfd);
			exit(0);
		}
	}
}

void usage()
{
	printf("Usage: server -p [PORT]\n");
	printf("Begin a TCP server listening on the given port.\n");
    exit(-1);
}

uint32_t listen_server(uint32_t port)
{
	struct sockaddr_in sa;
	uint32_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
	if (listen(sockfd, MAX_CONN) != 0)
	{
		debug_log(FATAL, __FILE__, "Server failed to listen on socket %d\n", sockfd);
		exit(-1);
	}
	debug_log(INFO, __FILE__, "Server listening on port %d\n", port);
	return sockfd;
}
//this thread gets initialized for each new connection. it's purpose is to receive data from a client on connfd and 
//then send it to 
void work(void* arg)
{
	uint32_t connfd = *(uint32_t*) arg;
	char buf[BUF_SZ];
	ssize_t rcvd;
	for (;;)
	{
		memset(buf, 0, BUF_SZ);
		rcvd = read(connfd, buf, BUF_SZ);
		if (rcvd > 0)
		{
			debug_log(INFO, __FILE__, "client %d: %s\n", connfd, buf);
		}
		//then, here, sendto() every client
		for (int i = 0; i < num_conn; ++i) 
		{
			if (connfds[i] == connfd) //don't resend the clients message back to itself
			{
				debug_log(INFO, __FILE__, "Skipping sending message to client %d\n", connfds[i]);
				continue;
			}
			debug_log(INFO, __FILE__, "Sending to client %d\n", connfds[i]);
			sendto(connfds[i], (void*) buf, rcvd, 0, (struct sockaddr*) &clients[i], sizeof(clients[i]));
		}
		memset(buf, 0, BUF_SZ);
		if (strncmp("exit", buf, 4) == 0)
		{
			debug_log(WARN, __FILE__, "Disconnecting client from server\n");
			--num_conn;
			memset(buf, 0, BUF_SZ);
			close(connfd);
			return;
		}
		else if (rcvd == 0)
		{
			debug_log(WARN, __FILE__, "Client disconnect received -- closing connection\n");
			--num_conn;
			memset(buf, 0, BUF_SZ);
			close(connfd);
			return;
		}
	}
}
