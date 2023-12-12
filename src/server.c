#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

void usage() 
{
	printf("Usage: server -p [PORT]\n");
	printf("Begin a TCP server listening on the given port.\n");
}


int main(int argc, char** argv) 
{
	char* port;
	int arg;

	while((arg = getopt(argc, argv, "p:")) != -1) 
	{
		switch (arg)
		{
			case 'p':
				port = optarg;
				break;
			default:
				usage();
				break;
		}
	}
	printf("using port %s\n", port);

}

