CC = gcc
CFLAGS = -O1 -Wall -I .
LDLIBS = -lpthread -lm

PROGS =	echoservert_pre race norace deadlock nodeadlock

all: $(PROGS)

echoservert_pre: echoservert_pre.c echo_cnt.c sbuf.c csapp.c csapp.h
	$(CC) $(CFLAGS) $(LDLIBS) echoservert_pre.c echo_cnt.c sbuf.c csapp.c -o echoservert_pre.exe

race: race.c csapp.c csapp.h
	$(CC) $(CFLAGS) $(LDLIBS) race.c csapp.c -o race.exe

norace: norace.c csapp.c csapp.h
	$(CC) $(CFLAGS) $(LDLIBS) norace.c csapp.c -o norace.exe

deadlock: deadlock.c csapp.c csapp.h
	$(CC) $(CFLAGS) $(LDLIBS) deadlock.c csapp.c -o deadlock.exe

nodeadlock: nodeadlock.c csapp.c csapp.h
	$(CC) $(CFLAGS) $(LDLIBS) nodeadlock.c csapp.c -o nodeadlock.exe

clean:
	rm -f *.exe