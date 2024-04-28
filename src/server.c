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
#include "clog.h"

//the server port
static uint32_t port; 
//all of the file descriptors open by the server
uint32_t connfds[MAX_CONN];
//all of the clients the server knows about
struct sockaddr_in clients[MAX_CONN];
//the total number of connections right now
uint32_t num_conn = 0;
//the names of all the clients connected
char* client_names[MAX_CONN];

void handle_new_client(struct sockaddr_in* client)
{
	char rcv_buf[16];
	char send_buf[BUF_SZ];
	memset(rcv_buf, 0, sizeof(rcv_buf));
	memset(send_buf, 0, BUF_SZ);
	//first receive the client's name, which is the first thing they do when connecting
	ssize_t rcvd = read(connfds[num_conn], rcv_buf, sizeof(rcv_buf));
	//register the new client into client_names
	client_names[num_conn] = strdup(rcv_buf);
	for (int i = 0; i <= num_conn; ++i)
	{
		strcat(send_buf, client_names[i]);
		strcat(send_buf, ",");
	}
	sendto(connfds[num_conn], &send_buf, BUF_SZ, 0, (struct sockaddr*) &clients[num_conn], sizeof(clients[num_conn]));
}

void update_clients()
{
	char send_buf[BUF_SZ];
	for (int i = 0; i < num_conn; ++i)
	{
		memset(send_buf, 0, BUF_SZ);
		strcat(send_buf, client_names[num_conn]);
		socklen_t client_len = sizeof(clients[num_conn]);
		sendto(connfds[i], &send_buf, BUF_SZ, 0, (struct sockaddr*) &clients[i], client_len);
	}
}

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
        clog(FATAL,"No port provided.\n");
        usage();
    }
	if (!(port = atoi(s_port)))
    {
        clog(FATAL,"Unable to parse port into number.\n");
        usage();
    }

	//bind to a socket and listen to it for incoming connections
	sockfd = listen_server(port);
	set_print_color(WARN);
	printf("Server running on port %d!\n", port);
	set_print_color(DEFAULT);
	while (num_conn < MAX_CONN) 
	{
		struct sockaddr_in client;
		socklen_t client_len = sizeof(client);
		if (!(connfds[num_conn] = accept(sockfd, (struct sockaddr*) &client, &client_len))) 
		{
			clog(WARN,"Failed to accept connection from client %d: %s", num_conn, strerror(errno));
		}
		clog(INFO,"New client connected on fd: %d\n", connfds[num_conn]);
		//right after connecting, the client will send it's name, so be ready to receive it
		handle_new_client(&client);
		//now that the server knows about the new client, update all the existing clients
		update_clients();
		//spawn thread to handle this connection
		pthread_t t;
		if ((pthread_create(&t, NULL, (void*) work, (void*) &connfds[num_conn]))) 
		{
			clog(WARN,"Failed to spawn delegate thread\n");
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
		clog(FATAL,"Failed to open TCP server socket\n");
		exit(-1);
	}
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(port);

	if ((bind(sockfd, (struct sockaddr*)&sa, sizeof(sa))) != 0)
	{
		clog(FATAL,"Server failed to bind to socket %d\n", sockfd);
		exit(-1);
	}
	if (listen(sockfd, MAX_CONN) != 0)
	{
		clog(FATAL,"Server failed to listen on socket %d\n", sockfd);
		exit(-1);
	}
	return sockfd;
}
//worker thread to handle the connection with a client. receives data from the client and distributes it to the rest of the clients
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
			clog(INFO, "%s\n", buf);
		}
		//then, here, sendto() every client
		for (int i = 0; i < num_conn; ++i) 
		{
			sendto(connfds[i], (void*) buf, rcvd, 0, (struct sockaddr*) &clients[i], sizeof(clients[i]));
		}
		memset(buf, 0, BUF_SZ);
		if (strncmp("exit", buf, 5) == 0)
		{
			clog(WARN,"Disconnecting client from server\n");
			--num_conn;
			memset(buf, 0, BUF_SZ);
			close(connfd);
			return;
		}
		else if (rcvd == 0)
		{
			clog(WARN,"Client disconnect received -- closing connection\n");
			--num_conn;
			memset(buf, 0, BUF_SZ);
			close(connfd);
			return;
		}
	}
}
