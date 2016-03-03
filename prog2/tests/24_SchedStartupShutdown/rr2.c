#include <lwp.h>
#include <stdlib.h>
#include <stdio.h>
#include <rr.h>

static thread qhead=NULL;
static int advance=FALSE;
#define tnext sched_one
#define tprev sched_two


static void rr_admit(thread new) {
  printf("Admitting new thread to Second scheduler\n");
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
  printf("Removing thread from Second scheduler\n");
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
  printf("(second->next())\n");

  if ( qhead ) {
    if ( advance  )
      qhead = qhead->tnext;
    else
      advance = TRUE;
  }

  return qhead;
}

void init() {
  printf("%s: called on second scheduler\n",__FUNCTION__);
}

void shutdown () {
  printf("%s: called on second scheduler\n",__FUNCTION__);
}

static struct scheduler rr_publish = {init, shutdown, rr_admit,rr_remove,rr_next};
scheduler AltAltRoundRobin = &rr_publish;


