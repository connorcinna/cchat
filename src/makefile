CC=gcc
CLFAGS=-I.
DEPS = server.h

server: server.c server.h
	$(CC) -o server server.c
client: client.c
	$(CC) -o client client.c
common: common.c common.h
	$(CC) -c -o common.o common.c
all: server client common
clean: 
	rm *.o server client
