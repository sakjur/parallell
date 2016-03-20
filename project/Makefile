CC=clang
CFLAGS=-std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O3

all: seq_sq.out par_sq.out

seq_sq.out: seq_sq.c gravn_common.o
	$(CC) $(CFLAGS) -lm -o $@ $< gravn_common.o

par_sq.out: par_sq.c
	$(CC) $(CFLAGS) -lm -lpthread -o $@ $< gravn_common.o

gravn_common.o: gravn_common.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) ./*.out
	$(RM) ./*.o

