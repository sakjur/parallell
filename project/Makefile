CC=clang
CFLAGS=-std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O3

all: seq_sq.out

seq_sq.out: seq_sq.c
	$(CC) $(CFLAGS) -lm -o $@ $<

clean:
	$(RM) ./*.out
	$(RM) ./*.o

