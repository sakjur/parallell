CC=gcc
CFLAGS=-std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O3

all: matrix

matrix: matrixSum-openmp.out matrixminmax.out

matrixSum-openmp.out: matrixSum-openmp.c
	$(CC) $(CFLAGS) -fopenmp -o $@ $<

matrixminmax.out: matrixminmax.c
	$(CC) $(CFLAGS) -fopenmp -o $@ $<

libmatrix.o: libmatrix.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) ./*.out
	$(RM) ./*.o

