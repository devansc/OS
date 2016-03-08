CC = gcc
ARGS = -g

all: minls minget

minget: minget.c minlib.o minprint.o
	$(CC) $(ARGS) -o minget minget.c minlib.o minprint.o

minls: minls.c minlib.o minprint.o
	$(CC) $(ARGS) -o minls minls.c minlib.o minprint.o

minlib.o: minlib.h minlib.c 
	$(CC) $(ARGS) -c -o minlib.o minlib.c

minprint.o: minprint.c minprint.h
	$(CC) $(ARGS) -c -o minprint.o minprint.c

clean:
	rm -f *.o minls minget
