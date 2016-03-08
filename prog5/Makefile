CC = gcc
ARGS = 

all: minls

minls: minls.c minlib.o minprint.o
	$(CC) $(ARGS) -o minls minls.c minlib.c minprint.c

minlib.o: minlib.h minlib.c 
	$(CC) $(ARGS) -c -o minlib.o minlib.c

minprint.o: minprint.c minprint.h
	$(CC) $(ARGS) -c -o minprint.o minprint.c

clean:
	rm -f *.o minls minget
