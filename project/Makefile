CC=clang
CFLAGS=-std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O0

all: seq_sq.out seq_sq.debug.out par_sq.out par_sq.debug.out seq_nlg.out \
	seq_nlg.debug.out

seq_sq.out: seq_sq.c gravn_common.o
	$(CC) $(CFLAGS) -lm -o $@ $< gravn_common.o

seq_sq.debug.out: seq_sq.c gravn_common.o
	$(CC) $(CFLAGS) -DDEBUG_MODE -lm -lpthread -o $@ $< gravn_common.o

par_sq.out: par_sq.c gravn_common.o
	$(CC) $(CFLAGS) -lm -lpthread -o $@ $< gravn_common.o

par_sq.debug.out: par_sq.c gravn_common.o
	$(CC) $(CFLAGS) -DDEBUG_MODE -lm -lpthread -o $@ $< gravn_common.o

seq_nlg.out: seq_nlg.c gravn_common.o
	$(CC) $(CFLAGS) -lm -o $@ $< gravn_common.o

seq_nlg.debug.out: seq_nlg.c gravn_common.o
	$(CC) $(CFLAGS) -DDEBUG_MODE -lm -lpthread -o $@ $< gravn_common.o

gravn_common.o: gravn_common.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) ./*.out
	$(RM) ./*.o

