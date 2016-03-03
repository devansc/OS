CC 	= gcc

CFLAGS  = -Wall -g -I .

LD 	= gcc

LDFLAGS  = -Wall -g -L.

PROGS	= liblwp.so

PROGOBJS  = lwp.o scheduler.o

OBJS	= $(PROGOBJS) 

all: 	$(PROGS)

clean:	
	rm -f $(OBJS) *~ TAGS liblwp.so

nums: numbersmain.o liblwp.so lwp.o
	$(LD) $(LDFLAGS) -o nums numbersmain.o -L. -llwp

numbermain.o: lwp.h numbersmain.c
	$(LD) $(LDFLAGS) -c numbersmain.c

liblwp.so: lwp.h lwp.c scheduler.c
	gcc -g -c -fPIC lwp.c magic64.S scheduler.c
	gcc -g -shared -fPIC -o liblwp.so lwp.o magic64.S scheduler.o

#libLWP.a: lwp.h lwp.c
#	gcc -c -g lwp.c magic64.S
#	ar r libLWP.a lwp.o magic64.o
#	rm lwp.o
