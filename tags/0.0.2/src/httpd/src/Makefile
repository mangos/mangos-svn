# Makefile for Null httpd
CC      = gcc
CFLAGS  = -Wall -O2 -s -I./include
LDFLAGS = -lpthread
OBJECTS = cgi.o config.o files.o format.o http.o main.o server.o

all:
	rm -f *.o
	make httpd

httpd: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o ../httpd/bin/httpd
	@echo Make httpd is complete.

clean:
	rm -f ../httpd/bin/httpd *.o *~
