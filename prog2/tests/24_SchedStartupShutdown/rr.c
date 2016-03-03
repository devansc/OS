#include <lwp.h>
#include <stdlib.h>
#include <stdio.h>
#include <rr.h>

static thread qhead=NULL;
static int advance=FALSE;
#define tnext sched_one
#define tprev sched_two


static void rr_admit(thread new) {
  printf("Admitting new thread to First scheduler\n");

  /* add to queue */
  if ( qhead ) {
    new->tnext = qhead;
    new->tprev = qhead->tprev;
    new->tprev->tnext = new;
    qhead->tprev = new;
  } else {
    advance = FALSE;
    qhead = new;
    qhead->tnext = new;
    qhead->tprev = new;
  }

}

static void rr_remove(thread victim) {
  printf("Removing thread from First scheduler\n");
  /* cut out of queue */
  victim->tprev->tnext = victim->tnext;
  victim->tnext->tprev = victim->tprev;

  /* what if it were qhead? */
  if ( victim == qhead ) {
    if ( victim->tnext != victim)
      qhead = victim->tprev;  /* preserve who would've been next */
    else
      qhead = NULL;
  }
}

static thread rr_next() {
  printf("(first->next())\n");

  if ( qhead ) {
    if ( advance  )
      qhead = qhead->tnext;
    else
      advance = TRUE;
  }

  return qhead;
}

static void init() {
  printf("%s: called on first scheduler\n",__FUNCTION__);
}

static void shutdown () {
  printf("%s: called on first scheduler\n",__FUNCTION__);
}

static struct scheduler rr_publish = {init, shutdown, rr_admit,rr_remove,rr_next};
scheduler AltRoundRobin = &rr_publish;

/*********************************************************/
__attribute__ ((unused)) void
dpl() {
  thread l;
  if ( !qhead )
    fprintf(stderr,"qhead is NULL\n");
  else {
    fprintf(stderr,"queue:\n");
    l = qhead;
    do {
      fprintf(stderr,"  (tid=%lu tnext=%p tprev=%p)\n", l->tid,l->tnext,
              l->tprev);
      l=l->tnext;
    } while ( l != qhead ) ;
    fprintf(stderr,"\n");
  }
}

