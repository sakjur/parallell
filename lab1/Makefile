CC=clang
CFLAGS=-std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O3

all: qsort.out matrix teeparty.out

matrix: matrixjoin.out matrixbag.out matrixminmax.out 

qsort.out: qsort.c
	$(CC) $(CFLAGS) -lpthread -o $@ $<

teeparty.out: teeparty.c
	$(CC) $(CFLAGS) -lpthread -o $@ $<

matrixminmax.out: matrixminmax.c libmatrix.o
	$(CC) $(CFLAGS) -lpthread -o $@ $< libmatrix.o

matrixbag.out: matrixbag.c libmatrix.o
	$(CC) $(CFLAGS) -lpthread -o $@ $< libmatrix.o

matrixjoin.out: matrixjoin.c libmatrix.o
	$(CC) $(CFLAGS) -lpthread -o $@ $< libmatrix.o

libmatrix.o: libmatrix.c
	$(CC) $(CFLAGS) -c -o $@ $<

