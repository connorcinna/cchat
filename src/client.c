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
#include <pthread.h>
#include "common.h"

struct sockaddr_in client;
struct sockaddr_in server;
static int port; 
static int sockfd;

void usage(void)
{
    printf("Usage: client -p [port] -s [server address] [-n] [name]\n");
    printf("Port and Address are required arguments. If you have not run this program before, a name must be provided as well.");
    printf("If you HAVE run this program before, /etc/cchat/config contains your cached username.");
	exit(-1);
}

void work(void) 
{
	char buf[BUF_SZ];
	for(;;)
	{
		memset(buf, 0, BUF_SZ);
		ssize_t bytes_read = read(STDIN_FILENO, buf, BUF_SZ);
		if (bytes_read > 255)
		{
			//TODO: split up the message into chunks
			debug_log(INFO, __FILE__, "Message too long -- discarding\n");
			continue;
		}
		buf[bytes_read] = '\0';
		sendto(sockfd, (void*) buf, sizeof(buf), 0, (struct sockaddr*) &server, sizeof(server));
		chat_print(buf);
	}
}
//TODO: make this work for the current client too, not just the remote one
void read_name(char* name) 
{
//	char name[BUF_SZ];
	FILE* f = fopen("/etc/cchat/config", "r");
	if (!f)
	{
		debug_log(INFO, __FILE__, "/etc/cchat/config does not exist.\n");
	}
	while (!feof(f))
	{
		fread(name, BUF_SZ, 1, f);
	}
	fclose(f);
}
//TODO: this doesnt seem to work yet
void write_name(char* name) 
{
	FILE* f = fopen("/etc/cchat/config", "w");
	if (!f)
	{
		debug_log(WARN, __FILE__, "Unable to write username to file - failed to open file!");
	}
	fprintf(f, name);
	debug_log(INFO, __FILE__, "Writing %s to cache\n");
	fclose(f);
}

static void read_resp(void* arg) 
{
	char buf[BUF_SZ];
	char* name = (char*) arg;
	ssize_t rcvd;
	for(;;) 
	{
		memset(buf, 0, BUF_SZ);
		rcvd = read(sockfd, buf, BUF_SZ);
		if (rcvd > 0) 
		{
			//debug_log(INFO, __FILE__, "from server: %s", buf);
			chat_print("%s: %s", name, buf);
		}
	}
}

int main(int argc, char** argv) 
{
	char* s_port = NULL;
	char* s_addr = NULL;
	char* name = NULL;
	int arg;

	while((arg = getopt(argc, argv, "n:p:s:")) != -1) 
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
			case 'n':
				name = optarg;
				debug_log(INFO, __FILE__, "Setting name to %s\n", name);
			default: 

		}
	}
    if (!s_port) 
    {
        debug_log(FATAL, __FILE__, "No port passed as arg.\n");
		usage();
    }
    if (!(port = atoi(s_port))) 
    {
        debug_log(FATAL, __FILE__, "Unable to parse port as number.\n");
		usage();
    }
	if (!s_addr) 
    {
        debug_log(FATAL, __FILE__, "No server address passed as arg.\n");
		usage();
    }
	if (!name)
	{
		debug_log(WARN, __FILE__, "No name passed in -- checking cache\n");
		read_name(name);
		if (!name)
		{
			debug_log(FATAL, __FILE__, "No name found in cache either -- unable to login\n");
			usage();		
		}
		else 
		{
			debug_log(INFO, __FILE__, "Writing name to cache for later\n");
			write_name(name);
		}
	}

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(s_addr);

    if (!(sockfd = socket(AF_INET, SOCK_STREAM, 0))) 
    {
        debug_log(FATAL, __FILE__, "Unable to open socket: %s\n", strerror(errno));
        exit(-1);
    }
	debug_log(INFO, __FILE__, "Attempting to connect to server: %s:%s\n", s_addr, s_port);
    if (connect(sockfd, (struct sockaddr*) &server, sizeof(server)))
    {
		debug_log(FATAL, __FILE__, "Unable to connect to server: %s\n", strerror(errno));
        exit(-1);
    }
	else 
	{
		debug_log(INFO, __FILE__, "Successfully connected to server %s with port %d\n", s_addr, port);
	}
	//have another thread read the return bytes from the server, so clients can see what other clients type
	pthread_t t;
	if ((pthread_create(&t, NULL, (void*) read_resp, (void*) name))) 
	{
		debug_log(WARN, __FILE__, "Failed to spawn delegate thread\n");
	}
	//do client stuff -- should this be a thread?
	work();
}
