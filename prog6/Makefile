CC = gcc
ARGS = -pedantic -Wall -ansi

all: hello

hello: hello.c
	$(CC) $(ARGS) -o hello hello.c

test: hello
	@ echo "Testing hello..."
	@ ./hello
	@ echo "done"

clean:
	rm -f hello
