CC=gcc
CFLAGS=-O2 -Wall
LDLIBS=-lpthread

all: echoclient echoserverp

echoclient: 
	$(CC) $(CFLAGS) echoclient.c csapp.c $(LDLIBS) -o echoclient.exe

echoserverp: 
	$(CC) $(CFLAGS) echoserverp.c echo.c csapp.c $(LDLIBS) -o echoserverp.exe

clean:
	rm -f *.exe