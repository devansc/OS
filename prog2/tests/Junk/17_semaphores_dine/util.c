#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <lwp.h>

void dawdle() {
  /*
   * yield() a random number of times to simulate delay
   */
  int num,i;
  for(i=0,num=random() & 0xFF; i<num;i++)
    lwp_yield();
}

void *safe_malloc ( int size ) {
  void *new;
  new = malloc(size);
  if ( new == NULL ) {
    perror(__FUNCTION__);
    exit(-1);
  }
  return new;
}
