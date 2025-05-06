# Makefile for echoclient and echoserver

CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lpthread

SRCDIR = .
TARGETS = echoclient echoserver

all: $(TARGETS)

echoclient: $(SRCDIR)/echoclient.c $(SRCDIR)/csapp.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

echoserver: $(SRCDIR)/echoserver.c $(SRCDIR)/csapp.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGETS)
