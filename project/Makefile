CC=gcc
CFLAGS=-std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O3 -lm

all: seq_sq.out seq_sq.debug.out par_sq.out par_sq.debug.out seq_nlg.out \
	seq_nlg.debug.out par_nlg.out par_nlg.debug.out

seq_sq.out: seq_sq.c gravn_common.o
	$(CC) $(CFLAGS) -o $@ $< gravn_common.o

seq_sq.debug.out: seq_sq.c gravn_common.o
	$(CC) $(CFLAGS) -DDEBUG_MODE -o $@ $< gravn_common.o

par_sq.out: par_sq.c gravn_common.o
	$(CC) $(CFLAGS) -lpthread -o $@ $< gravn_common.o

par_sq.debug.out: par_sq.c gravn_common.o
	$(CC) $(CFLAGS) -DDEBUG_MODE -lpthread -o $@ $< gravn_common.o

seq_nlg.out: seq_nlg.c gravn_common.o
	$(CC) $(CFLAGS) -o $@ $< gravn_common.o

seq_nlg.debug.out: seq_nlg.c gravn_common.o
	$(CC) $(CFLAGS) -DDEBUG_MODE -o $@ $< gravn_common.o

par_nlg.out: par_nlg.c gravn_common.o
	$(CC) $(CFLAGS) -lpthread -o $@ $< gravn_common.o

par_nlg.debug.out: par_nlg.c gravn_common.o
	$(CC) $(CFLAGS) -DDEBUG_MODE -lpthread -o $@ $< gravn_common.o

gravn_common.o: gravn_common.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) ./*.out
	$(RM) ./*.o

