#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lwp.h"
#include "schedulers.h"

#define MAXSNAKES  100
#define INITIALSTACK 2048
#define NUMTHREADS 9
#define ITERS 50

#define tnext sched_one
#define tprev sched_two

typedef void (*sigfun)(int signum);
static void indentnum(uintptr_t num);

int main(int argc, char *argv[]){
  long i;

  srandom(0);                   /* so it's repeatable */
  lwp_set_scheduler(Random);

  printf("Launching LWPS\n");

  /* spawn a number of individual LWPs */
  for(i=1;i<=NUMTHREADS;i++) {
    lwp_create((lwpfun)indentnum,(void*)i,INITIALSTACK);
  }

  lwp_start();                     /* returns when the last lwp exits */

  printf("Back from LWPS.\n");
  return 0;
}

extern thread randomGetHead(void);
static void checktids() {
  /* Check that for every thread in the scheduler tid2thread
   * works
   */
  printf("Checking thread IDs...\n");
  thread t,head,tail;

  head=randomGetHead();
  if ( !head ) {
    fprintf(stderr,"Where's the thread list?\n");
    exit(-1);
  } else {
    tail=head->tnext;
    do {
      t = tid2thread(tail->tid);
      if ( t != tail ) {
        printf("  tid2thread() returned the wrong pointer (tid=%ld)\n",
                tail->tid);
      }
      tail = tail->tnext;
    } while (tail != head);
  }
  printf("...done.\n");
}

static void indentnum(uintptr_t num) {
  /* print the number num num times, indented by 5*num spaces
   * Not terribly interesting, but it is instructive.
   */
  int howfar,i;

  howfar=(int)num;              /* interpret num as an integer */
  for(i=0;i<ITERS;i++){
    printf("%*d\n",howfar*5,howfar);
    if ( num == NUMTHREADS && i == ITERS - 3 ) {
      checktids();
      exit(EXIT_SUCCESS);
    }
    lwp_yield();                /* let another have a turn */
  }
  lwp_exit();                   /* bail when done.  This should
                                 * be unnecessary if the stack has
                                 * been properly prepared
                                 */
}

