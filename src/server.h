#ifndef SERVER_H
#define SERVER_H

//prints usage information
void usage(void);

//bind to a socket and listen to it for incoming connections
//returns a file descriptor to the socket being listened on
uint32_t listen_server(uint32_t port);

//handle the connection from a client
//continuously receive packets from the client and read them to another client
static void work(void* arg);

#endif
