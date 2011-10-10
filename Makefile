CC=gcc
CFLAGS=-O3

all:	lzw
lzw:	lzw.o bit_io.o bit_io.h byte_io.o byte_io.h
	$(CC) $(CFLAGS) -o lzw lzw.o bit_io.o byte_io.o
lzw.o:	lzw.c
	$(CC) $(CFLAGS) -c lzw.c
bit_io.o:	bit_io.c
	$(CC) $(CFLAGS) -c bit_io.c
byte_io.o:	byte_io.c
	$(CC) $(CFLAGS) -c byte_io.c
clean:
	rm -f lzw lzw.o bit_io.o byte_io.o
