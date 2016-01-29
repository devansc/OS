#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <lwp.h>

#define STACKSIZE   4096
#define THREADCOUNT 4096
#define TARGETCOUNT 50000


int count;
int yieldcount;

static void body(int num)
{
  int i;

  for(;;) {
    int num = random()&0xF;
    for(i=0;i<num;i++) {
      yieldcount++;
      lwp_yield();
    }
    if ( count < TARGETCOUNT )
      count++;
    else
      lwp_exit();
  }
}


int main() {
  int i;

  srandom(0);                   /* There's random and there's random... */
  printf("Spawining %d threads.\n", THREADCOUNT);
  count=0;
  for(i=0;i<THREADCOUNT;i++)
    lwp_create((lwpfun)body,(void*)0,STACKSIZE);
  lwp_start();
  printf("Done.  Count is %d. (Yielded %d times)\n", count, yieldcount);
  exit(0);
}
