CC=gcc
CFLAGS=-g -O1 -Wall
LDLIBS=-lpthread

all: hostinfo

hostinfo:
	$(CC) $(CFLAGS) hostinfo.c csapp.c $(LDLIBS) -o hostinfo.exe

clean:
	rm -f *.exe