CC = gcc
CFLAGS = -O1 -Wall
PROGS =	sharing badcnt goodcnt
	
all: $(PROGS)

sharing: sharing.c csapp.c csapp.h
	$(CC) $(CFLAGS) sharing.c csapp.c -lpthread -o sharing.exe

badcnt: badcnt.c csapp.c csapp.h
	$(CC) $(CFLAGS) badcnt.c csapp.c -lpthread -o badcnt.exe
	$(CC) $(CFLAGS) -S badcnt.c

goodcnt: goodcnt.c csapp.c csapp.h
	$(CC) $(CFLAGS) goodcnt.c csapp.c -lpthread -o goodcnt.exe
	$(CC) $(CFLAGS) -S goodcnt.c

clean:
	rm -f *.exe *.s