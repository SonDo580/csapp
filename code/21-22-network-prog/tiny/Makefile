CC = gcc
CFLAGS = -O2 -Wall -I .
LIB = -lpthread

all: tiny cgi

tiny:
	$(CC) $(CFLAGS) tiny.c csapp.c $(LIB) -o tiny.exe

cgi:
	(cd cgi-bin; make)

clean:
	rm -f *.exe
	(cd cgi-bin; make clean)
