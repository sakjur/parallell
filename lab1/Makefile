CC=clang
CFLAGS=-std=c11 -g -Wall -D_XOPEN_SOURCE=600 -O3

all: matrixjoin.out

matrixjoin.out: matrixjoin.c libmatrix.o
	$(CC) $(CFLAGS) -lpthread -o $@ $< libmatrix.o

libmatrix.o: libmatrix.c
	$(CC) $(CFLAGS) -c -o $@ $<

