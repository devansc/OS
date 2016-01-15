SHELL = /bin/sh

CC 	= gcc

LD 	= gcc

FLAGS = -g -Wall -I../common -L../../lib -L../../lib64 -lmalloc -lbackstop

PROG 	= 03_malloc_use

REFFILE	= $(PROG).expected

LIBDIR 	= ../../lib

.PHONY: prog

all:  $(REFFILE).out 

prog: $(PROG)32 $(PROG)64

$(REFFILE).out: $(PROG)32 $(PROG)64
	@echo -n Regenerating $@...
	@LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH;\
	  export LD_LIBRARY_PATH;\
	  $(PROG)32 > $(REFFILE).out 2> $(REFFILE).err
	@echo done.

clean:
	-rm -f $(OBJS) *~

allclean: clean
	-rm -f $(PROG) $(REFFILE).out $(REFFILE).err

$(PROG)32: prog.c ../common/lib.c
	$(CC) -m32 $(FLAGS) -o $@ prog.c ../common/lib.c

$(PROG)64: prog.c ../common/lib.c
	$(CC) $(FLAGS) -o $@  prog.c ../common/lib.c
