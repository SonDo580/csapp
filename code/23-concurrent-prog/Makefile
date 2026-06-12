CC=gcc
CFLAGS=-O2 -Wall
LDLIBS=-lpthread

all: echoclient echoserverp echoservert

echoclient: 
	$(CC) $(CFLAGS) echoclient.c csapp.c $(LDLIBS) -o echoclient.exe

echoserverp: 
	$(CC) $(CFLAGS) echoserverp.c echo.c csapp.c $(LDLIBS) -o echoserverp.exe

echoservert: 
	$(CC) $(CFLAGS) echoservert.c echo.c csapp.c $(LDLIBS) -o echoservert.exe

clean:
	rm -f *.exe