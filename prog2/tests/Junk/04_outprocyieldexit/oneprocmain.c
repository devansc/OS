/*
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lwp.h"

#define INITIALSTACK 4096

static void indentnum(uintptr_t num) {
  /* print the number num num times, indented by 5*num spaces
   * Not terribly interesting, but it is instructive.
   */
  printf("Greetings from Thread %d.  Yielding...\n",(int)num);
  lwp_yield();
  printf("I'm still alive.  Goodbye.\n");
  lwp_exit(0);
}


int main(int argc, char *argv[]){
  long i;

  /* spawn a number of individual LWPs */
  for(i=0;i<1;i++)
    lwp_create((lwpfun)indentnum,(void*)i,INITIALSTACK);

  lwp_start();

  printf("LWPs have ended.\n");
  return 0;
}


