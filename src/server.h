#ifndef SERVER_H
#define SERVER_H
#define MAXCONN 10
#define BUFF_SZ 256

//prints usage information
void usage();

//bind to a socket and listen to it for incoming connections
//returns a file descriptor to the socket being listened on
int listen_server(int port);

//handle the connection from a client
//continuously receive packets from the client and read them to another client
void handle_connection(int connfd);

#endif
