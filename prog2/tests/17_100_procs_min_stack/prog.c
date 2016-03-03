#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <lwp.h>

#define STACKSIZE 40            /* should be enough to call lwp_exit() */

int count;

static void nothin(int num)
{
  count++;
  return;
}


int main() {
  int i;
  printf("Spawining 100 minumal threads.\n");
  count=0;
  for(i=0;i<100;i++)
    lwp_create((lwpfun)nothin,(void*)0,STACKSIZE);
  lwp_start();
  printf("Done.  Count is %d.\n", count);
  exit(0);
}
