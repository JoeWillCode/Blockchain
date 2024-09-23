CC = gcc
CFLAGS = -g -Wall -std=c99

all: addlog checklog clean

addlog: addlog.c
	$(CC) $(CFLAGS) -o $@ $< -lssl -lcrypto

checklog: checklog.c
	$(CC) $(CFLAGS) -o $@ $< -lssl -lcrypto

clean:
	rm -f *.o