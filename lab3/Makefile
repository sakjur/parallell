CC=clang
CFLAGS=-std=c99 -g -Wall -D_XOPEN_SOURCE=600 -O3

all: hungrybirds.out honeybees.out

hungrybirds.out: hungrybirds.c
	$(CC) $(CFLAGS) -lpthread -o $@ $<

honeybees.out: honeybees.c
	$(CC) $(CFLAGS) -lpthread -o $@ $<

clean:
	$(RM) ./*.out
	$(RM) ./*.o

