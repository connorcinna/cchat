CC=gcc -D_XOPEN_SOURCE_EXTENDED
CLFAGS=-I. -g
SRCDIR=$(CURDIR)/src
BUILDDIR=$(CURDIR)/bin
LIBDIR=$(CURDIR)/lib
LIBS=-lncursesw
LOGDIR=$(CURDIR)/log

dirs: 
	mkdir -p $(BUILDDIR) $(LIBDIR) $(LOGDIR)
list: $(SRCDIR)/list.c $(SRCDIR)/list.h
	$(CC) -c -o $(BUILDDIR)/list.o $(SRCDIR)/list.c
server: $(SRCDIR)/server.c $(SRCDIR)/server.h $(SRCDIR)/common.h
	$(CC) -o $(BUILDDIR)/server $(SRCDIR)/server.c $(LIBDIR)/libcommon.o -pthread $(LIBS)
client: $(SRCDIR)/client.c 
	$(CC) -o $(BUILDDIR)/client $(SRCDIR)/client.c $(LIBDIR)/libcommon.o -pthread $(LIBS)
common: $(SRCDIR)/common.c $(SRCDIR)/common.h
	$(CC) -c -o $(LIBDIR)/libcommon.o $(SRCDIR)/common.c

all: dirs list common server client
clean: 
	rm -f $(BUILDDIR)/*
	rm -f $(LIBDIR)/*
	rm -f $(LOGDIR)/*
