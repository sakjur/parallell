CC=clang++
CFLAGS=-std=c++14 -g -Wall -D_XOPEN_SOURCE=600 -O3

all: fss.out

fss.out: fss.cpp
	$(CC) $(CFLAGS) -lpthread -o $@ $<

clean:
	$(RM) ./*.out
	$(RM) ./*.o

