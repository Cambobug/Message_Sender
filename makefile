CC = gcc

all: server sendFile

server: server.c network.h
	$(CC) server.c -g -o server -lm

sendFile: sendFile.c network.h
	$(CC) sendFile.c -g -o sendFile -lm

clean:
	rm server sendFile
