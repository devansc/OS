#include <lwp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sems.h"

#define tnext sched_one
#define tprev sched_two

/* General queues for use below */
typedef thread queue;           /* a pointer to a thread will do. */

static queue runQueue=NULL;     /* "the" run queue */


int rql() {
  /* return the length of the run queue */
  int len=0;
  thread t;

  if ( runQueue ) {
    t=runQueue;
    do {
      len++;
      t = t->tnext;
    } while (t != runQueue);
  }

  return len;
}


static thread head(queue *Q) {
  return *Q;
}



static void enqueue(thread T, queue *Q) {
  /* add to queue */
  if ( *Q ) {
    T->tnext = *Q;
    T->tprev = (*Q)->tprev;
    T->tprev->tnext = T;
    (*Q)->tprev = T;
  } else {
    (*Q) = T;
    (*Q)->tnext = T;
    (*Q)->tprev = T;
  }
}

static void q_remove(thread T, queue *Q) {
  /* remove this thread regardless of where it is in the queue */
  T->tprev->tnext = T->tnext;
  T->tnext->tprev = T->tprev;
  /* what if it was the head? */
  if ( T == *Q ) {
    if ( T->tnext != T)
      *Q = T->tprev;  /* preserve who would've been next */
    else
      *Q = NULL;
  }
}

static thread dequeue(queue *Q) {
  thread res;
  if ( (res = head(Q)) )
    q_remove(res,Q);
  return res;
}

/* now a semaphore type */

struct semaphore {
  char *name;                   /* a useful name for printing */
  int val;                      /* the value */
  queue Q;
};

Semaphore newsem(char *name, int num) {
  Semaphore new;
  char *n;

  new = malloc (sizeof(struct semaphore));
  n   = strdup(name);
  if ( new && n && num >= 0) {
    new->name = n;
    new->val = num;
    new->Q   = NULL;
  }
  return new;
}

void up(Semaphore s){
  thread wake;
  if ( (wake = dequeue(&s->Q)) ) {
    enqueue(wake,&runQueue);
  } else {
    s->val++;
  }
}

void down(Semaphore s) {
  thread t;
  if ( s->val ) {
    s->val--;
  } else {
    t = dequeue(&runQueue);     /* whatever's running  */
    if ( !t ) {
      fprintf(stderr, "%s: Empty run queue.  Then who called us?\n",
              __FUNCTION__);
      exit(1);
    }
    enqueue(t,&s->Q);
  }
  lwp_yield();
}


/* and now the scheduler itself... */

static void s_admit(thread new) {

  /* add to queue */
  enqueue(new,&runQueue);
}

static void s_remove(thread victim) {
  /* cut out of queue */
  q_remove(victim,&runQueue);
}

static thread s_next() {
  /* advance between zero and 256 threads */
  int i,num;

  if ( runQueue )
    for(i=0,num = random() & 0xFF; i<num;i++)
      runQueue = runQueue->tnext;

  return runQueue;
}

struct scheduler s_publish = {NULL, NULL, s_admit,s_remove,s_next};
scheduler Semaphores = &s_publish;

/*********************************************************/
__attribute__ ((unused)) void
dpl() {
  thread l;
  if ( !runQueue )
    fprintf(stderr,"runQueue is NULL\n");
  else {
    fprintf(stderr,"queue:\n");
    l = runQueue;
    do {
      fprintf(stderr,"  (tid=%lu tnext=%p tprev=%p)\n", l->tid,l->tnext,
              l->tprev);
      l=l->tnext;
    } while ( l != runQueue ) ;
    fprintf(stderr,"\n");
  }
}

__attribute__ ((unused)) void
pq (queue Q) {
  thread t;
  if ( Q ) {
    t=runQueue;
    do {
      printf("%p ",(void*)t->tid);
      t = t->tnext;
    } while (t != runQueue);
  }
  printf("\n");
}


