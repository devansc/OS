SHELL = /bin/sh

CC 	= gcc

LD 	= gcc

CFLAGS 	= -fPIC -g -Wall -I$(LIBDIR) -I../shared

LDFLAGS = -g -Wall -L$(LIBDIR) -llwp

OBJS 	= sems.o

LIBDIR 	= ../../lib

all:	libshared.a

libshared.a: $(OBJS)
	ar r libshared.a $(OBJS)

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
