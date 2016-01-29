SHELL = /bin/sh

CC 	= gcc

LD 	= gcc

CFLAGS 	= -g -Wall -I$(LIBDIR)

LDFLAGS = -g -Wall -L$(LIBDIR) -llwp

PROG = 13_StopRestart

OBJS 	= stop_start.o

REFFILE	= $(PROG).expected

LIBDIR 	= ../../lib

all:	$(REFFILE)

prog: $(PROG)

$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(REFFILE): $(PROG) 
	@echo -n Regenerating $@...
	@LD_LIBRARY_PATH=$(LIBDIR):$$LD_LIBRARY_PATH;\
	export LD_LIBRARY_PATH;\
	$(PROG) > $(REFFILE)
	@echo done.

clean:
	-rm -f $(OBJS) *~

allclean: clean
	-rm -f $(PROG) $(REFFILE)
