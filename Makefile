#Makefile k IPK projektu

CC=g++
CFLAGS=-std=c++98 -pedantic -Wextra -Wall

all: client server

client:
	$(CC) $(CFLAGS) client.cpp -o client
server:
	$(CC) $(CFLAGS) server.cpp -o server	
clean:
	rm server
	rm client
