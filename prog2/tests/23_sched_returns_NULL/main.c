
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lwp.h"


extern scheduler Weird;

#define INITIALSTACK 2048

static void threadfun(uintptr_t num) {
  printf("First print in threadfun().\n");
  lwp_yield();
  printf("This print shouldn't happen.\n");
}


int main(int argc, char *argv[]){
  long i=0;

  printf("Before the LWPS.\n");

  lwp_set_scheduler(Weird);
  lwp_create((lwpfun)threadfun,(void*)i,INITIALSTACK);
  lwp_start();                     /* returns when the last lwp exits */

  printf("Back from LWPS.\n");
  return 0;
}

