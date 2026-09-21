CC = gcc
CFLAGS = -g -Og -I../include

CSAPP_HDR = ../include/csapp.h
CSAPP_SRC = ../include/csapp.c
CSAPP_OBJ = csapp.o

PROGS = shellex forks sigint sigintsafe
EXES = $(PROGS:=.exe)

.PHONY: all clean $(PROGS)

all: $(EXES)

$(PROGS): %: %.exe

$(EXES): %.exe: %.c $(CSAPP_OBJ)
	$(CC) $(CFLAGS) $< $(CSAPP_OBJ) -o $@

$(CSAPP_OBJ): $(CSAPP_SRC) $(CSAPP_HDR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.exe *.o