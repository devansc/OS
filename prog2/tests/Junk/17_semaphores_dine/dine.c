/*
 * dine.c is the core of a pthreads implementation of the
 * Dining Philosophers Problem to illustrate interprocess
 * communication.
 *
 * Revision History:
 *
 * $Log: dine.c,v $
 * Revision 1.3  2003-02-04 15:06:36-08  pnico
 * Prepared the relase version with "extra" explanation
 * for use as a class example.
 *
 *
 */
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <sems.h>
#include "util.h"
#include "scoreboard.h"

#define STACKSIZE 8192

/*
 * encode the program's ID in the executable.  Usefull if you
 * ever need to identify the file using strings(1)
 */
const char *version="$Id: dine.c,v 1.3 2003-02-04 15:06:36-08 pnico Exp $";

static void unimportant_initialization();

int needed_thoughts=5;          /* default to thinking five times */

#define NUM_PHILOSOPHERS 5
#define LEFT(N)  (N)
#define RIGHT(N) ((N+1)%NUM_PHILOSOPHERS)

Semaphore *dinnerfork;            /* initialized in main() */

void think(int who) {
  /* change status to thinking, hang out, then move on to the
   * next activity.
   */
  sb_change_status(who,THINKING);
  dawdle();
  sb_change_status(who,WAITING);
}

void eat(int who) {
  /* change status to eating, hang out, then move on to the
   * next activity.
   *   Assumes the calling philosopher is holding both forks.
   */
  sb_change_status(who,EATING);
  dawdle();
  sb_change_status(who,WAITING);
}

void take_fork(int which, int who) {

  /* pick up the given fork, then report having it to
   * the world.  This is safe, since no other philosopher
   * could report having the fork if it happens to get
   * interrupted between the two statements.
   */

  down(dinnerfork[which]);
  sb_takefork(who,which);
}

void release_fork(int which, int who) {
  /* Report the fork as dropped, then unlock its semaphore.
   * This is safe, because even if there is a context switch
   * in-between, another process could not pick up the fork
   * until the semaphore is unlocked.
   */
  sb_dropfork(who,which);
  up(dinnerfork[which]);
}

void philosopher(void *id) {
  /*
   * philosopher() is the function that implements the life of
   * a philisopher.  It takes its ID as loops the given number of times
   * through the eat-think cycle.
   */
  int who = *(int*)id;
  int num;
  int first,second;

  /* We will break the symmetry below (and prevent deadlock) by
   * forcing odd-numbered philosophers to taker the left fork
   * first, and even numbered ones to take the right one first
   */
  if ( who % 2 == 1 ) {         /* odd numbered philosopher */
    first  = LEFT(who);
    second = RIGHT(who);
  } else {                     /* even-numbered */
    first  = RIGHT(who);
    second = LEFT(who);
  }

  /* Cycle through the eat-think cycle the requisite number
   * of times.
   */
  for (num = 0; num < needed_thoughts; num++) {
    /* take the forks.      */
    take_fork(first,who);
    take_fork(second,who);

    /* eat for a while ("Grub first, then ethics" --- B. Brecht) */
    eat(who);

    /* release the forks */
    release_fork(first,who);
    release_fork(second,who);

    /* now think a bit. */
    think(who);
  }
}


int main(int argc, char *argv[]){
  /* create NUM_PHILOSOPHERS mutexes (one for each fork) and
   * NUM_PHILOSOPHERS threads (one for each philosopher)
   * start them going.
   *
   * Recall that with pointer arithmetic, p+i is equivalent to
   * &(p[i])
   */
  int i;
  int *id;

  if ( argc == 2 ) {
    if ( 1 != sscanf(argv[1],"%d",&needed_thoughts) ) {
      fprintf(stderr,"usage: %s [times around]\n",argv[0]);
      exit(-1);
    }
  } else if (argc > 2) {
    fprintf(stderr,"usage: %s [times around]\n",argv[0]);
    exit(-1);
  }

  srandom(0);                    /* there's random and there's random... */
  lwp_set_scheduler(Semaphores); /* kinda important, eh? */

  sb_init(NUM_PHILOSOPHERS);

  dinnerfork = (Semaphore*)safe_malloc(NUM_PHILOSOPHERS *
                                             sizeof(Semaphore));
  id         = (int*)safe_malloc(NUM_PHILOSOPHERS * sizeof(int));

  for(i=0;i<NUM_PHILOSOPHERS;i++){
    char name[100];
    snprintf(name, sizeof(name), "fork %d", i);
    dinnerfork[i] = newsem(name,1);
  }


  for(i = 0; i < NUM_PHILOSOPHERS; i++) {
    id[i]=i;
    if (-1 == lwp_create(philosopher,
                             (void *)(id+i),
                             STACKSIZE)) {
      fprintf(stderr,"thread %i: %s\n",i,
              strerror(errno));
    }
  }

  lwp_start();

  /* shut down the scoreboard */
  sb_destroy();

  return 0;
}

