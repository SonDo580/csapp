CC = gcc
CFLAGS = -O2 -Wall -I ..

all: adder

adder: adder.c
	$(CC) $(CFLAGS) adder.c ../csapp.c -o adder.exe

clean:
	rm -f *.exe