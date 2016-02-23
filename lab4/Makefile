CC=clang++
CFLAGS=-std=c++14 -g -Wall -D_XOPEN_SOURCE=600 -O3
DEPS=spacestation.h

all: fss.out

fss.out: fss.cpp spacestation.o vehicle.o spacestation.h
	$(CC) $(CFLAGS) -lpthread -o $@ $< spacestation.o vehicle.o

%.o: %.cpp $(DEPS)
		$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(RM) ./*.out
	$(RM) ./*.o

