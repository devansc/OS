#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lwp.h"
#include "schedulers.h"

#define MAXSNAKES  100
#define INITIALSTACK 2048
#define NUMTHREADS 1
#define ITERS 1

#define tnext sched_one
#define tprev sched_two

typedef void (*sigfun)(int signum);
static void threadfun(uintptr_t num);

int main(int argc, char *argv[]){
  long i;

  srandom(0);                   /* so it's repeatable */
  lwp_set_scheduler(Random);

  printf("Launching LWPS\n");

  /* spawn a number of individual LWPs */
  for(i=1;i<=NUMTHREADS;i++) {
    lwp_create((lwpfun)threadfun,(void*)i,INITIALSTACK);
  }

  lwp_start();                     /* returns when the last lwp exits */

  printf("Back from LWPS.\n");
  return 0;
}

extern thread randomGetHead(void);
static void threadfun(uintptr_t num) {
  /* Get our own tid, then call tid2thread with something that
   * isn't it.
   */
  thread t,head;
  head=randomGetHead();
  tid_t tid = head->tid;
  t = tid2thread(tid+1);
  if ( t ) {
    fprintf(stderr,
            "tid2thread() returned a value with a bogus tid.  Not good\n");
  }

  lwp_exit();                   /* bail when done.  This should
                                 * be unnecessary if the stack has
                                 * been properly prepared
                                 */
}

