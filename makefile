CC=gcc
CLFAGS=-I.
LDFLAGS= -L $(LIBDIR) -l libcommon.o
DEPS = server.h
SRCDIR=$(CURDIR)/src
BUILDDIR=$(CURDIR)/bin
LIBDIR=$(CURDIR)/lib
LOGDIR=$(CURDIR)/log

dirs: 
	mkdir -p $(BUILDDIR) 
	mkdir -p $(LIBDIR)
	mkdir -p $(LOGDIR)
server: $(SRCDIR)/server.c $(SRCDIR)/server.h
	$(CC) -o $(BUILDDIR)/server $(SRCDIR)/server.c $(LIBDIR)/libcommon.o
client: $(SRCDIR)/client.c
	$(CC) -o $(BUILDDIR)/client $(SRCDIR)/client.c $(LIBDIR)/libcommon.o
common: $(SRCDIR)/common.c $(SRCDIR)/common.h
	$(CC) -c -o $(LIBDIR)/libcommon.o $(SRCDIR)/common.c

all: dirs common server client
clean: 
	rm -f $(BUILDDIR)/*
	rm -f $(LIBDIR)/*
