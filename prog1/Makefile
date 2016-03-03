all: malloc libmalloc intel-all

malloc: malloc.h malloc.c
	gcc -g -Wall -fPIC -ansi -c malloc.c

libmalloc: malloc.o
	gcc -shared -fPIC -o libmalloc.so malloc.o
	ar r libmalloc.a malloc.o

intel-all: malloc.o
	mkdir lib
	gcc -Wall -g -fpic -m32 -c -o malloc32.o malloc.c
	gcc -Wall -g -fpic -m32 -shared -o lib/libmalloc.so malloc32.o
	mkdir lib64
	gcc -Wall -g -fpic -c -o malloc64.o malloc.c
	gcc -Wall -g -fpic -shared -o lib64/libmalloc.so malloc64.o

clean:
	rm -f *.o libmalloc.so
	rm -rf lib lib64
