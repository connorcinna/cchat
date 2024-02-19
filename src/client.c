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
#include <signal.h>
#include "client.h"

//network info of this client
struct sockaddr_in client;
//network info of the server
struct sockaddr_in server;
//string for holding the address of the server we're connected to 
static char addr_str[INET_ADDRSTRLEN];
//the port to connect to the server on
static uint16_t port; 
//the socket we are connect()'ed to
static uint32_t sockfd;
//the name of this client
char* name = NULL;
//path to config file -- currently only holds a cached name if a user has logged in before
static char* config_path;
//a list of peers that the client knows about -- used for populating the side window
static char* peers[MAX_CONN];
//the number of peers currently known
static uint32_t peer_count = 0;
////the main window where chat between clients is displayed
WINDOW* win_main;
//side window where all of the client's names will appear
WINDOW* win_clients;
//small bottom window where the user will type their message
WINDOW* win_msg;
//dimensions of the entire terminal screen
uint32_t max_y, max_x;
//dimensions of the main chat window
uint32_t main_y, main_x = 0;
//dimensions of the message window
uint32_t msg_y, msg_x = 0;
//dimensions of the client window
uint32_t clients_y, clients_x = 0;
//the current row that the client prints to -- has to be global
uint32_t row = 1;

void size_warning(void)
{
	log(ERROR, "Window not large enough: Must be at least 16x10\n");
}
//initialize windows or handle resize events
void determine_win_size(void)
{
	getmaxyx(stdscr, max_y, max_x);
	if (max_y < 10 || max_x < 16)
	{
		size_warning();
		return;
	}

	if ((wresize(win_main, max_y  - 5, max_x - 16)) == ERR)
	{
		log(ERROR, "Failed to resize win_main\n");
	}
	if ((wresize(win_msg, 5, max_x - 16)) == ERR)
	{
		log(ERROR, "Failed to resize win_msg\n");
	}
	if ((wresize(win_clients, max_y, 16)) == ERR)
	{
		log(ERROR, "Failed to resize win_msg\n");
	}

	getmaxyx(win_main, main_y, main_x);
	getmaxyx(win_msg, msg_y, msg_x);
	getmaxyx(win_clients, clients_y, clients_x);
}
//signal handler
void handle_signal(int signal)
{
	switch (signal)
	{
		case SIGINT:
			log(ERROR, "Interrupt signal received -- shutting down\n");
			endwin();	
			exit(0);
			break;
		case SIGWINCH:
			log(INFO, "Resize signal received\n");
			determine_win_size();
			redraw_main();
			redraw_msg();
			redraw_clients();
			log(INFO, "new window sizes:\n");
			log(INFO, "main_x: %d main_y: %d\nmsg_x: %d msg_y: %dclients_x: %d clients_y: %d\n", main_x, main_y, msg_x, msg_y, clients_x, clients_y);
			log(INFO, "max_x: %d, max_y: %d\n", max_x, max_y);
	}
}

//print information regarding how to start the program from commandline and exit
void usage(void)
{
    printf("Usage: client -p [port] -s [server address] [-n] [name]\n");
    printf("Port and Address are required arguments. If you have not run this program before, a name must be provided as well.");
    printf("If you HAVE run this program before, ~/.config/etc/config contains your cached username.");
	exit(-1);
}
void redraw_main(void)
{
	box(win_main, ACS_VLINE, ACS_HLINE);
	wmove(win_main, 0, (main_x / 2));
	wprintw(win_main, "CCHAT -- %s", addr_str);
}
void redraw_msg(void)
{
	box(win_msg, ACS_VLINE, ACS_HLINE);
	wmove(win_msg, 1, 1);
	wprintw(win_msg, "%s: ", name);
}
void redraw_clients(void)
{
	box(win_clients,ACS_VLINE, ACS_HLINE);
	wmove(win_clients, 0, (clients_x / 2)-1);
	wprintw(win_clients, "ROOM");
}
//busy work for setting up structs or initializing variables
//TODO: handle resizing
void init_ncurses(void) 
{
	initscr();
	getmaxyx(stdscr, max_y, max_x);
	//have to initialize signal handlers after initscr()
	signal(SIGINT, handle_signal);	
	signal(SIGWINCH, handle_signal);	

	win_main = newwin(max_y - 5, max_x - 16, 0, 16);
    win_msg = newwin(5, max_x - 16, max_y - 5, 16);
	win_clients = newwin(max_y, 16, 0, 0);

	getmaxyx(win_main, main_y, main_x);
	getmaxyx(win_msg, msg_y, msg_x);
	getmaxyx(win_clients, clients_y, clients_x);

	scrollok(win_main, TRUE);
	scrollok(win_msg, TRUE);
	scrollok(win_clients, TRUE);

	wsetscrreg(win_main, 1, main_y-1);
	idlok(win_main, TRUE);
	leaveok(win_main, TRUE);
	nocbreak();

	redraw_main();
	redraw_msg();
	redraw_clients();

	wmove(win_msg, 1, 1);

	if (has_colors())
	{
		start_color();
		init_pair(1, COLOR_GREEN, COLOR_BLACK);
	}

	wbkgd(win_main, COLOR_PAIR(1));
	wbkgd(win_msg, COLOR_PAIR(1));
	wbkgd(win_clients, COLOR_PAIR(1));

	wrefresh(win_main);
	wrefresh(win_clients);
	wrefresh(win_msg);

}
void init(char* s_addr, char* s_port) 
{
	memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(s_addr);
	if (!(sockfd = socket(AF_INET, SOCK_STREAM, 0))) 
    {
        log(FATAL,"Unable to open socket: %s\n", strerror(errno));
        exit(-1);
    }
    if (connect(sockfd, (struct sockaddr*) &server, sizeof(server)))
    {
		log(FATAL,"Unable to connect to server: %s\n", strerror(errno));
        exit(-1);
    }
	
	//register our name with the server
	sendto(sockfd, (void*) peers[0], sizeof(peers[0]), 0, (struct sockaddr*) &server, sizeof(server));

	inet_ntop(server.sin_family, &(((struct sockaddr_in*)&server)->sin_addr), addr_str, INET_ADDRSTRLEN);
	init_ncurses();
	char rcv_buf[BUF_SZ];
	//read the server's response in the format "name,name,name,name"
	memset(rcv_buf, 0, BUF_SZ);
	read(sockfd, rcv_buf, BUF_SZ);
	//the first string is read by this one, and we check if theres more
	char* pch;
	pch = strtok(rcv_buf, ",");
	wmove(win_clients, 1, 1);
	wprintw(win_clients, "%s", pch);
	wrefresh(win_clients);
	peers[0] = strdup(pch);
	++peer_count;
	while (pch = strtok(NULL, ","))
	{
		wmove(win_clients, peer_count + 1, 1);
		wprintw(win_clients, "%s", pch);
		wrefresh(win_clients);
		peers[peer_count] = strdup(pch);
		++peer_count;
	}

	redraw_clients();

	wrefresh(win_clients);
}

void work(void) 
{
	char buf[BUF_SZ];
	char tmp[BUF_SZ];
	//format for a message:
	//name + ": " + msg 
	//so the message size can be BUF_SZ - name size - 2
	uint32_t max_msg_sz = BUF_SZ - (strlen(name)) - 2;

	for(;;)
	{
		memset(buf, 0, BUF_SZ);
		wmove(win_msg, 1, 1);
		wprintw(win_msg, "%s: ", name);
		wmove(win_msg, 1, strlen(name) + 3);
		wgetnstr(win_msg, buf, max_msg_sz);	
		//ncurses is weird on windows. this is the best we're getting to signal handling
		if (!strcmp(buf, "exit") || strstr(buf, "^C"))
		{
			raise(SIGINT);
		}
		wclear(win_msg);
		size_t bytes_read = strlen(buf);
		if (bytes_read > max_msg_sz)
		{
			//TODO: split up the message into chunks
			log(ERROR,"Message too long -- discarding\n");
			continue;
		}
		memset(tmp, 0, BUF_SZ);
		strcat(tmp, name);
		strcat(tmp, ": ");
		strcat(tmp, buf);

		redraw_msg();

		wrefresh(win_main);
		wrefresh(win_msg);
		//send "name: msg" back to server
		sendto(sockfd, (void*) tmp, BUF_SZ, 0, (struct sockaddr*) &server, sizeof(server));
	}
}
char* read_name(char* name) 
{
	FILE* f = fopen(config_path, "r");
	if (!f)
	{
		log(INFO,"%s does not exist.\n", config_path);
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
		log(WARN,"Unable to write username to file: %s\n", strerror(errno));
	}
	if(fprintf(f, "%s", name) < 0)
	{
		log(WARN,"Unable to write username to file : %s\n", strerror(errno));
	}
	else
	{
		log(INFO,"Writing %s to cache\n", name);
	}
	fclose(f);
}
void handle_resp(void) 
{
	char buf[BUF_SZ];
	//buffer of empty spaces to clear out the line that we're about to print on
	//necessary for making sure the box() drawn from last row is erased
	uint32_t clr_buf_sz = main_x - 2;
	char clr_buf[clr_buf_sz];
	for (uint32_t i = 0; i < clr_buf_sz; ++i)
	{
		clr_buf[i] = ' ';	
	}
	clr_buf[clr_buf_sz] = '\0';
	for(;;) 
	{
		memset(buf, 0, BUF_SZ);
		ssize_t rcvd = read(sockfd, buf, BUF_SZ);
		char* tmp = strdup(buf);
		strtok(tmp, ":");
		//see if we already know about this peer. if flag == 1, we know about it already
		int32_t flag = 0;
		for (uint32_t i = 0; i < peer_count; ++i) 
		{
			if (!strcmp(peers[i], tmp)) //equal
			{
				flag = 1;	
			}
		}
		if (!flag)
		{
			peers[peer_count] = strdup(tmp);
			++peer_count;
			wmove(win_clients, peer_count, 1);
			wprintw(win_clients, "%s\n", tmp);

			redraw_clients();	

			wrefresh(win_clients);

			continue;
		}
		wmove(win_main, row, 1);
		wprintw(win_main, "%s", clr_buf);

		wmove(win_main, row, 1);
		wprintw(win_main,"%s", buf);

		redraw_main();	

		wmove(win_msg, 1, 1);
		wprintw(win_msg, "%s: ", name);


		if (row < main_y - 2) //minus 2 for the size of the border
		{
			++row;
		}
		else
		{
			scroll(win_main);
			redraw_main();
			wmove(win_main, row, 1);
			wprintw(win_main, "%s", clr_buf);
		}
		wmove(win_msg, 1, strlen(name) + 3);
		wrefresh(win_main);
		wrefresh(win_msg);
	}
}

int main(int argc, char** argv) 
{
#ifdef __linux__
	char* home = getenv("HOME");
	strcat(home,  "/.config/cchat/config");
	config_path = strdup(home);
#elif _WIN32
	config_path = "/mnt/c/projects/cchat/config";
#else 
#endif
	char* s_port = NULL;
	char* s_addr = NULL;
	int arg;

	while((arg = getopt(argc, argv, "n:p:s:")) != -1) 
	{
		switch (arg)
		{
			case 'p': 
				s_port = optarg;
				break;
			case 's':
				s_addr = optarg;
				break;
			case 'n':
				name = optarg;
			default: 
		}
	}
    if (!s_port || !(port = atoi(s_port))) 
    {
        log(FATAL,"No port passed as arg.\n");
		usage();
    }
	if (!s_addr) 
    {
        log(FATAL,"No server address passed as arg.\n");
		usage();
    }
	if (!name)
	{
		log(WARN,"No name passed in -- checking cache\n");
		name = read_name(name);
		if (!name)
		{
			log(FATAL,"No name found in cache either -- unable to login\n");
			usage();		
		}
		else 
		{
			write_name(name);
		}
	}
	else
	{
		write_name(name);
	}
	peers[0] = strdup(name);
	//get stuff set up
	init(s_addr, s_port);
	//have another thread read the return bytes from the server, so clients can see what other clients type
	pthread_t t;
	if ((pthread_create(&t, NULL, (void*) handle_resp, NULL))) 
	{
		log(WARN,"Failed to spawn delegate thread\n");
	}
	//do client stuff 
	work();
	endwin();
}
