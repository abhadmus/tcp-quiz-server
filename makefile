# Makefile for Quiz Server and Client
# ./server 127.0.0.1 8080
# ./client 127.0.0.1 8080
# cd "/home/asb/unix assignment 3"

CC = gcc
CFLAGS = -Wall -Wextra -g

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client
