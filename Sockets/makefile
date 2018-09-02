CC=gcc

all: server client

server:
	$(CC) -o server server.c -pthread
	
client:
	$(CC) -o client client.c

clean:
	rm server client