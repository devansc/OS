#include "lwp.h"
#include <stdio.h>
#define firstContext sched_one
#define lastContext sched_two

struct scheduler rrScdlr = {NULL, NULL, rr_admit, rr_remove, rr_next};
scheduler rrScheduler = &rrScdlr;

void rr_admit(thread newThread) {
    if (systemThread->firstContext == NULL) {
        systemThread->firstContext = systemThread->lastContext = newThread;
        systemThread->nextThread = newThread;
        newThread->nextThread = newThread->lastThread = newThread;
        return;
    }

    newThread->nextThread = systemThread->firstContext;
    newThread->lastThread = systemThread->lastContext;
    systemThread->lastContext->nextThread = newThread;
    systemThread->firstContext->lastThread = newThread;
    systemThread->lastContext = newThread;
}

void rr_remove(thread victim) {
    victim->lastThread->nextThread = victim->nextThread;
    victim->nextThread->lastThread = victim->lastThread;
    if (victim == systemThread->firstContext) {
        if (victim->nextThread != victim)
            systemThread->nextThread = systemThread->firstContext = 
                    victim->nextThread;
        else
            systemThread->nextThread = systemThread->firstContext =
                    systemThread->lastThread = NULL;
    }
}

thread rr_next() {
    /*
    if (!curThread)
        return NULL;
    curThread = curThread->nextThread;
    return curThread;
    */
    return curThread ? curThread->nextThread : NULL;
}
