#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <lwp.h>
#include <sems.h>

#define STACKSIZE   (8192*16)
#define TIMES 10

int report_ids;

Semaphore ping, pong;

static void ponger(int times)
{
  int i;
  tid_t id = lwp_gettid();

  for(i=0;i<times; i++ ) {
    down(pong);
    if ( report_ids )
      printf("...pong (%ld).\n",id);
    else
      printf("...pong.\n");
    up(ping);
  }
}

static void pinger(int times)
{
  int i;
  tid_t id = lwp_gettid();

  for(i=0;i<times; i++ ) {
    down(ping);
    if ( report_ids )
      printf("   Ping (%ld)", id);
    else
      printf("   Ping");
    up(pong);
  }
}


int main() {
  int i;

  srandom(0);                   /* There's random and there's random... */

  report_ids = (getenv("VERBOSE") != NULL);
  lwp_set_scheduler(Semaphores);
  ping = newsem("Ping",1);
  pong = newsem("Pong",0);

  for(i=0;i<TIMES;i++) {
    lwp_create((lwpfun)ponger,(void*)2,STACKSIZE);
    lwp_create((lwpfun)pinger,(void*)2,STACKSIZE);
  }

  printf("Spawining threads. (%d)\n",rql());
  lwp_start();
  printf("Done.\n");
  exit(0);
}
