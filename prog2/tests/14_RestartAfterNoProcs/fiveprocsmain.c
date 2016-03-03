/*
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lwp.h"

#define INITIALSTACK 4096

static void indentnum(uintptr_t num) {
  /* print the number num num times, indented by 5*num spaces
   * Not terribly interesting, but it is instructive.
   */
  int id;
  id = (int)num;
  printf("Greetings from Thread %d.  Yielding...\n",id);
  lwp_yield();
  printf("I (%d) am still alive.  Goodbye.\n",id);
  lwp_exit();
}


int main(int argc, char *argv[]){
  long i;

  /* spawn a number of individual LWPs */
  for(i=0;i<5;i++)
    lwp_create((lwpfun)indentnum,(void*)i,INITIALSTACK);

  lwp_start();
  printf("First round complete. Restarting with no queue\n");
  lwp_start();
  printf("Creating five LWPs and starting them\n");

  /* spawn a number of individual LWPs */
  for(i=0;i<5;i++)
    lwp_create((lwpfun)indentnum,(void*)i,INITIALSTACK);

  lwp_start();

  printf("LWPs have ended.\n");
  return 0;
}


