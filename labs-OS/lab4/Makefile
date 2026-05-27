# makefile for thread pool

CC=gcc
CFLAGS=-Wall
PTHREADS=-lpthread

all: client.o threadpool.o
	$(CC) $(CFLAGS) -o example client.o threadpool.o $(PTHREADS)

client.o: client.c threadpool.h
	$(CC) $(CFLAGS) -c client.c $(PTHREADS)

threadpool.o: threadpool.c threadpool.h
	$(CC) $(CFLAGS) -c threadpool.c $(PTHREADS)

clean:
	rm -rf *.o
	rm -rf example