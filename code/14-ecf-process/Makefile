CC=gcc
CFLAGS = -g -O2 -I.
LDLIBS = -lpthread

PROGS = fork

all: (PROGS)

fork: fork.c csapp.c csapp.h 
	$(CC) $(CFLAGS) $(LDLIBS) fork.c csapp.c -o fork.exe

clean:
	rm -rf *.exe
