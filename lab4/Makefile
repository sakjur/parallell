CC=clang++
CFLAGS=-std=c++11 -g -Wall -D_XOPEN_SOURCE=600 -O3
DEPS=spacestation.hpp

all: fss.out

fss.out: fss.cpp spacestation.o vehicle.o spacestation.hpp
	$(CC) $(CFLAGS) -lpthread -o $@ $< spacestation.o vehicle.o

%.o: %.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	$(RM) ./*.out
	$(RM) ./*.o

