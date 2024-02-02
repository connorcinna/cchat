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
#include <ncurses.h>
#include "common.h"

struct sockaddr_in client;
struct sockaddr_in server;
static int port; 
static int sockfd;
static char* config_path;

void usage(void)
{
    printf("Usage: client -p [port] -s [server address] [-n] [name]\n");
    printf("Port and Address are required arguments. If you have not run this program before, a name must be provided as well.");
    printf("If you HAVE run this program before, ~/.config/etc/config contains your cached username.");
	exit(-1);
}

void initialize(char* s_addr, char* s_port) 
{
	initscr();
	nocbreak();
	noecho();
	scrollok(stdscr, TRUE);
	if (has_colors())
	{
		start_color();
		init_pair(1, COLOR_BLACK, COLOR_GREEN);
	}
	wbkgd(stdscr, COLOR_PAIR(1));
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
	refresh();
}

void work(char* name) 
{
	char buf[BUF_SZ];
	for(;;)
	{
		memset(buf, 0, BUF_SZ);
		ssize_t bytes_read = read(STDIN_FILENO, buf, BUF_SZ);
		//format:
		//name + ": " + msg 
		//so the message size can be BUF_SZ - name size - 2
		if (bytes_read > (BUF_SZ - (sizeof(name) + 2) ))
		{
			//TODO: split up the message into chunks
			debug_log(INFO, __FILE__, "Message too long -- discarding\n");
			continue;
		}
		char tmp[BUF_SZ];
		strcat(tmp, name);
		strcat(tmp, ": ");
		strcat(tmp, buf);
		//locally, print out our own information
		wprintw(stdscr,"%s: %s\n", name, buf);
		refresh();
		//send "name: msg" back to server
		sendto(sockfd, (void*) tmp, BUF_SZ, 0, (struct sockaddr*) &server, sizeof(server));
	}
}
char* read_name(char* name) 
{
	FILE* f = fopen(config_path, "r");
	if (!f)
	{
		debug_log(INFO, __FILE__, "%s does not exist.\n", config_path);
		return name;
	}
	char* line;
	size_t len;
	char* new_name;
	if (getline(&line, &len, f) != -1) 
	{
		new_name = strdup(line);
	}
	if (line) 
	{
		free(line);
	}
	return new_name;
	fclose(f);
}

void write_name(char* name) 
{
	FILE* f = fopen(config_path, "w");
	if (!f)
	{
		debug_log(WARN, __FILE__, "Unable to write username to file: %s\n", strerror(errno));
	}
	if(fprintf(f, "%s", name) < 0)
	{
		debug_log(WARN, __FILE__, "Unable to write username to file : %s\n", strerror(errno));
	}
	else
	{
		debug_log(INFO, __FILE__, "Writing %s to cache\n", name);
	}
	fclose(f);
}

static void read_resp() 
{
	char buf[BUF_SZ];
	ssize_t rcvd;
	for(;;) 
	{
		memset(buf, 0, BUF_SZ);
		rcvd = read(sockfd, buf, BUF_SZ);
		wprintw(stdscr,"%s\n", buf);
		refresh();
	}
}

int main(int argc, char** argv) 
{
	char* home = getenv("HOME");
	strcat(home,  "/.config/cchat/config");
	config_path = strdup(home);

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
		name = read_name(name);
		if (!name)
		{
			debug_log(FATAL, __FILE__, "No name found in cache either -- unable to login\n");
			usage();		
		}
		else 
		{
			debug_log(INFO, __FILE__, "Writing %s to cache for later%d\n", name, __LINE__);
			write_name(name);
		}
	}
	else
	{
		debug_log(INFO, __FILE__, "Writing %s to cache for later%d\n", name, __LINE__);
		write_name(name);
	}
	//get stuff set up
	initialize(s_addr, s_port);
	//do client stuff -- should this be a thread?
	//have another thread read the return bytes from the server, so clients can see what other clients type
	pthread_t t;
	if ((pthread_create(&t, NULL, (void*) read_resp, NULL))) 
	{
		debug_log(WARN, __FILE__, "Failed to spawn delegate thread\n");
	}
	work(name);
	endwin();
}
