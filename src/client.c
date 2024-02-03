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

//this client
struct sockaddr_in client;
//the server
struct sockaddr_in server;
//the main window where chat between clients is displayed
WINDOW* win_main;
//side window where all of the client's names will appear
WINDOW* win_clients;
//small bottom window where the user will type their message
WINDOW* win_msg;
//the port to connect to the server on
static int port; 
//the socket we are connect()'ed to
static int sockfd;
//path to config file -- currently only holds a cached name if a user has logged in before
static char* config_path;
//the maximum size of the terminal
uint32_t max_y, max_x;
//the current row that the client prints to -- has to be global
uint32_t row = 1;

//print information regarding how to start the program from commandline and exit
void usage(void)
{
    printf("Usage: client -p [port] -s [server address] [-n] [name]\n");
    printf("Port and Address are required arguments. If you have not run this program before, a name must be provided as well.");
    printf("If you HAVE run this program before, ~/.config/etc/config contains your cached username.");
	exit(-1);
}
//busy work for setting up structs or initializing variables
//TODO: do some fancy math to figure out how big these windows need to be instead of using magic numbers
//TODO: currently, user can't see what they're typing. the cursor moves to the win_msg window, but characters don't show up as they type
//it can show up with raw(), but that fucks up how often it prints to the screen
void init_ncurses(void) 
{
	initscr();
	getmaxyx(stdscr, max_y, max_x);
	win_clients = newwin(max_y, 10, 0, 0);
	win_msg = newwin(5, max_x - 10, max_y - 5, 10);
	win_main = newwin(max_y-5, max_x-10, 0, 10);
	box(win_clients, '|', '-');
	box(win_msg, '|', '-');
	box(win_main, '|', '-');
	nocbreak();
	scrollok(win_main, TRUE);
	scrollok(win_msg, TRUE);
	scrollok(win_clients, TRUE);
	if (has_colors())
	{
		start_color();
		init_pair(1, COLOR_GREEN, COLOR_BLACK);
	}
	wbkgd(win_main, COLOR_PAIR(1));
	wbkgd(win_clients, COLOR_PAIR(1));
	wbkgd(win_msg, COLOR_PAIR(1));
	wmove(win_msg, 1, 1);
	wrefresh(win_main);
	wrefresh(win_clients);
	wrefresh(win_msg);
}
void init(char* s_addr, char* s_port) 
{
	init_ncurses();
	memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(s_addr);

    if (!(sockfd = socket(AF_INET, SOCK_STREAM, 0))) 
    {
        debug_log(FATAL, __FILE__, "Unable to open socket: %s\n", strerror(errno));
        exit(-1);
    }
    if (connect(sockfd, (struct sockaddr*) &server, sizeof(server)))
    {
		debug_log(FATAL, __FILE__, "Unable to connect to server: %s\n", strerror(errno));
        exit(-1);
    }
	
}
void work(char* name) 
{
	char buf[BUF_SZ];
	char tmp[BUF_SZ];
	for(;;)
	{
		memset(buf, 0, BUF_SZ);
		ssize_t bytes_read = read(STDIN_FILENO, buf, BUF_SZ);
		wrefresh(win_main);
		//format:
		//name + ": " + msg 
		//so the message size can be BUF_SZ - name size - 2
		if (bytes_read > (BUF_SZ - (sizeof(name) + 2) ))
		{
			//TODO: split up the message into chunks
			debug_log(INFO, __FILE__, "Message too long -- discarding\n");
			continue;
		}
		memset(tmp, 0, BUF_SZ);
		strcat(tmp, name);
		strcat(tmp, ": ");
		strcat(tmp, buf);
		wmove(win_main, row, 1);
		wprintw(win_main, "%s", tmp);
		wmove(win_msg, 1, 1);
		wrefresh(win_main);
		wrefresh(win_msg);
		//send "name: msg" back to server
		sendto(sockfd, (void*) tmp, BUF_SZ, 0, (struct sockaddr*) &server, sizeof(server));
		if (row <= max_x)
		{
			++row;
		}
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
	char* peers[MAX_CONN];
	for(;;) 
	{
		memset(buf, 0, BUF_SZ);
		ssize_t rcvd = read(sockfd, buf, BUF_SZ);
		char* tmp = strdup(buf);
		strtok(tmp, ":");
		debug_log(INFO, __FILE__, "%s\n", tmp);
		uint32_t index = 0;
		for (int i = 0; i < MAX_CONN; ++i) 
		{
			if (peers[i] == tmp)
			{
				index = i;
			}
		}
		if (!index)
		{
			peers[index] = tmp;	
			wprintw(win_clients, "%s\n", tmp);
			wrefresh(win_clients);
		}
		wmove(win_main, row, 1);
		wprintw(win_main,"%s", buf);
		wrefresh(win_main);
		if (row <= max_x)
		{
			++row;
		}
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
			write_name(name);
		}
	}
	else
	{
		debug_log(INFO, __FILE__, "Writing %s to cache for later\n", name);
		write_name(name);
	}
	//get stuff set up
	init(s_addr, s_port);
	//have another thread read the return bytes from the server, so clients can see what other clients type
	pthread_t t;
	if ((pthread_create(&t, NULL, (void*) read_resp, NULL))) 
	{
		debug_log(WARN, __FILE__, "Failed to spawn delegate thread\n");
	}
	//do client stuff 
	work(name);
	endwin();
}
