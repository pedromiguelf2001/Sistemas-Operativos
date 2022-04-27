CC = gcc
CFLAGS = -g

all: sdstore sdstored

sdstore: sdstore.o

sdstore.o: sdstore.c

sdstored: sdstored.o

sdstored.o: sdstored.c

clean:
	rm -f *sdstore sdstored *.o
	rm -f tmp/c2s_fifo
