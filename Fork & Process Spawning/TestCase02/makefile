CC=gcc
CFLAGS=-std=c99
DBFLAGS=-gcc

make: votecounter.c
	$(CC) $(CFLAGS) -o votecounter votecounter.c

debug: votecounter.c
	$(CC) $(CFLAGS) $(DBFLAGS) -o votecounter votecounter.c

clean:
	rm votecounter
	rm Output_*