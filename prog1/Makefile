all: malloc libmalloc testMalloc

malloc: malloc.h malloc.c
	gcc -g -Wall -ansi -c malloc.c

libmalloc: malloc.o
	gcc -shared -fpic -o libmalloc.so malloc.o

testMalloc: malloc.o testMalloc.c libmalloc.so
	gcc testMalloc.c -L. -lmalloc -o testMalloc

clean:
	rm malloc.o libmalloc.so testMalloc
