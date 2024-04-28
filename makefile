CC=gcc -D_XOPEN_SOURCE_EXTENDED
SRCDIR=$(CURDIR)/src
BUILDDIR=$(CURDIR)/bin
LIBDIR=$(CURDIR)/lib
INC=-I$(CURDIR)/include
LIBS=-lncursesw
LOGDIR=$(CURDIR)/log

dirs: 
	mkdir -p $(BUILDDIR) $(LIBDIR) $(LOGDIR)
server: $(SRCDIR)/server.c $(SRCDIR)/server.h 
	$(CC) $(INC) -o $(BUILDDIR)/server $(SRCDIR)/server.c $(LIBDIR)/clog.o -pthread $(LIBS) 
client: $(SRCDIR)/client.c $(SRCDIR)/client.h
	$(CC) $(INC) -o $(BUILDDIR)/client $(SRCDIR)/client.c $(LIBDIR)/clog.o -pthread $(LIBS) 
all: dirs server client
clean: 
	rm -f $(BUILDDIR)/*
	rm -f $(LOGDIR)/*
