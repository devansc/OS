SHELL = /bin/sh

CC 	= gcc

LD 	= gcc

FLAGS = -g -Wall -I../common -L../../lib -L../../lib64 -lmalloc -lbackstop

PROG 	= 01_linkit_malloc 

REFFILE	= $(PROG).expected

LIBDIR 	= ../../lib

all:  $(REFFILE).out 

prog: $(PROG)32 $(PROG)64

$(REFFILE).out: $(PROG)32 
	@echo -n Regenerating $@...
	@LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH;\
	  export LD_LIBRARY_PATH;\
	  $(PROG) > $(REFFILE).out 2> $(REFFILE).err
	@echo done.

clean:
	-rm -f $(OBJS) *~

allclean: clean
	-rm -f $(PROG) $(REFFILE)

$(PROG)32: prog.c ../common/lib.c
	$(CC) -m32 $(FLAGS) -o $@

$(PROG)64: prog.c ../common/lib.c
	$(CC) $(FLAGS) -o $@
