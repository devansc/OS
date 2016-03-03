CC 	= gcc

SHELL   = /bin/sh

CFLAGS  = -Wall -g -fpic 

AR	= ar r 

RANLIB  = ranlib

ARCHIVE	= libmalloc.a

SO	= libmalloc.so

OBJS	= malloc.o 

SRCS	= malloc.c

HDRS	= 

EXTRACLEAN = core libmalloc.a libmalloc.so lib lib64 malloc32.o malloc64.o

.PHONY: all shared archive allclean clean malloc intel-all test

all: 	shared archive

malloc: shared archive

shared: $(SO)

archive: $(ARCHIVE)

intel-all: lib/$(SO) lib64/$(SO)

lib/$(SO): lib malloc32.o 
	$(CC) $(CFLAGS) -m32 -shared -o $@ malloc32.o

lib64/$(SO): lib64 malloc64.o
	$(CC) $(CFLAGS) -shared -o $@ malloc64.o

lib: 
	mkdir lib

lib64:
	mkdir lib64

allclean: clean
	@rm -rf $(EXTRACLEAN)

clean:	
	rm -f $(OBJS) *~ TAGS

$(ARCHIVE): $(OBJS)
	$(AR) $@ $(OBJS)
	ranlib $@

$(SO): $(OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(OBJS)

malloc32.o: malloc.c
	$(CC) $(CFLAGS) -m32 -c -o malloc32.o malloc.c

malloc64.o: malloc.c
	$(CC) $(CFLAGS) -m64 -c -o malloc64.o malloc.c

depend:
	@echo Regenerating local dependencies.
	@makedepend -Y $(SRCS) $(HDRS)

tags : $(SRCS) $(HDRS)
	etags $(SRCS) $(HDRS)  

test: intel-all
	~pn-cs453/demos/tryAsgn1 | tee Test.log 2>&1

# DO NOT DELETE

