CC 	= gcc

CFLAGS  = -Wall -g -I .

LD 	= gcc

LDFLAGS  = -Wall -g -L.

PROGS	= nums

NUMOBJS    = numbersmain.o

OBJS	= $(NUMOBJS) liblwp.so lwp.o

all: 	$(PROGS)

clean:	
	rm -f $(OBJS) *~ TAGS

nums: numbersmain.o liblwp.so
	$(LD) $(LDFLAGS) -o nums numbersmain.o -L. -llwp

numbermain.o: lwp.h numbersmain.c
	$(LD) $(LDFLAGS) -c numbersmain.c

liblwp.so: lwp.h lwp.c
	gcc -g -c -fPIC lwp.c magic64.S
	gcc -g -shared -fPIC -o liblwp.so lwp.o magic64.S

#libLWP.a: lwp.h lwp.c
#	gcc -c -g lwp.c magic64.S
#	ar r libLWP.a lwp.o magic64.o
#	rm lwp.o

#libPLN.a: ../Publish/lwp.c
#	gcc -c ../Publish/lwp.c
#	ar r libPLN.a lwp.o
#	rm lwp.o

