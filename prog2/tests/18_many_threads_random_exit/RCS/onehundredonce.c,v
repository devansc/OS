head	1.1;
access;
symbols;
locks; strict;
comment	@ * @;


1.1
date	2013.04.07.19.19.52;	author pnico;	state Exp;
branches;
next	;


desc
@Changed new_lwp() to lwp_create()
@


1.1
log
@Initial revision
@
text
@#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <lwp.h>

#define STACKSIZE 8096

static void counter(int num)
{
  if ( num ) {
    lwp_create((lwpfun)counter,(void*)num-1,STACKSIZE);
    printf("%d\n",num);
  } else {
    printf("Bye\n");
  }
  lwp_exit();
}


int main() {
  lwp_create((lwpfun)counter,(void*)100,STACKSIZE);
  lwp_start();
  exit(0);
}
@
