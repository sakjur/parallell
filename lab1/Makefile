CC=clang
CFLAGS=-std=c11 -g -Wall -D_XOPEN_SOURCE=600 -O3

all: matrixjoin.out matrixbag.out matrixminmax.out

matrixminmax.out: matrixminmax.c libmatrix.o
	$(CC) $(CFLAGS) -lpthread -o $@ $< libmatrix.o

matrixbag.out: matrixbag.c libmatrix.o
	$(CC) $(CFLAGS) -lpthread -o $@ $< libmatrix.o

matrixjoin.out: matrixjoin.c libmatrix.o
	$(CC) $(CFLAGS) -lpthread -o $@ $< libmatrix.o

libmatrix.o: libmatrix.c
	$(CC) $(CFLAGS) -c -o $@ $<

