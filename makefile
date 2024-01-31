CC=gcc
CLFAGS=-I. -g
SRCDIR=$(CURDIR)/src
BUILDDIR=$(CURDIR)/bin
LIBDIR=$(CURDIR)/lib
LOGDIR=$(CURDIR)/log

dirs: 
	mkdir -p $(BUILDDIR) $(LIBDIR) $(LOGDIR)
server: $(SRCDIR)/server.c $(SRCDIR)/server.h $(SRCDIR)/common.h
	$(CC) -o $(BUILDDIR)/server $(SRCDIR)/server.c $(LIBDIR)/libcommon.o -pthread
client: $(SRCDIR)/client.c 
	$(CC) -o $(BUILDDIR)/client $(SRCDIR)/client.c $(LIBDIR)/libcommon.o -pthread
common: $(SRCDIR)/common.c $(SRCDIR)/common.h
	$(CC) -c -o $(LIBDIR)/libcommon.o $(SRCDIR)/common.c

all: dirs common server client
clean: 
	rm -f $(BUILDDIR)/*
	rm -f $(LIBDIR)/*
	rm -f $(LOGDIR)/*
