CC=gcc
CFLAGS=-g -O1 -Wall
LDLIBS=-lpthread

all: hostinfo echoclient echoserver

hostinfo:
	$(CC) $(CFLAGS) hostinfo.c csapp.c $(LDLIBS) -o hostinfo.exe

echo: echoclient echoserver
echoclient: 
	$(CC) $(CFLAGS) echoclient.c csapp.c $(LDLIBS) -o echoclient.exe
echoserver: 
	$(CC) $(CFLAGS) echoserver.c csapp.c $(LDLIBS) -o echoserver.exe

clean:
	rm -f *.exe