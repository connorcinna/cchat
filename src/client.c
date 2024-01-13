#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

static int port; 
static struct sockaddr_in* server;
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
				break;
		}
	}
	printf("Attempting to connect to server: %s:%s", s_addr, s_port);
	getaddrinfo(s_addr, s_port, 0, &server);
	struct sockaddr_in* ptr;
	for (ptr = server; ptr != NULL; ptr = ptr->ai_next) 
	{
		if (ptr->ai_addr->sa_family == AF_INET)
		{
			server = ptr;	
			break;
		}
	}


}
