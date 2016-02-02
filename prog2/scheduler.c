#include "lwp.h"
#include <stdio.h>

#define nextThread sched_one 
#define prevThread sched_two 

struct scheduler rrScdlr = {NULL, NULL, rr_admit, rr_remove, rr_next};
scheduler rrScheduler = &rrScdlr;

thread headQ = NULL;
thread tailQ = NULL;
int started = 0;

void rr_admit(thread newThread) {
    if (headQ == NULL) {
        headQ = tailQ = newThread;
        newThread->nextThread = newThread->prevThread = newThread;
        return;
    }

    newThread->nextThread = headQ;
    newThread->prevThread = tailQ;
    tailQ->nextThread = newThread;
    headQ->prevThread = newThread;
    tailQ = newThread;
}

void rr_remove(thread victim) {
    victim->prevThread->nextThread = victim->nextThread;
    victim->nextThread->prevThread = victim->prevThread;
    started = 0;
    if (victim == headQ) {
        if (victim->nextThread != victim)
            headQ = victim->nextThread;
        else
            headQ = tailQ = NULL;
    }
}

thread rr_next() {
    if (!headQ)
        return NULL;
    if (!started) {
        started = 1;
        return headQ;
    }
    headQ = headQ->nextThread;
    return headQ;
}
