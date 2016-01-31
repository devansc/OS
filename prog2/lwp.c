#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lwp.h"


#define firstContext sched_one
#define lastContext sched_two
#define nextThread lib_one 
#define lastThread lib_two 
#define WORDSIZE sizeof(unsigned long)

struct scheduler rrScdlr = {NULL, NULL, rr_admit, rr_remove, rr_next};
scheduler rrScheduler = &rrScdlr;
scheduler currentScheduler = NULL;

static context systemContext;
thread systemThread = NULL;

tid_t nextTid = 1;
thread curThread = NULL;

void printStack(thread t) {
    printf("STACK: pos %p\n", t->stack);
    printf("\t%lu\n", t->stack[0]);
    printf("\t%lu\n", t->stack[-1]);
    printf("\t%lu\n", t->stack[-2]);
}

void printThreads(thread cur, tid_t last) {
    if (!cur || cur->tid < last) return;
    printf("\tthread %ld\n", cur->tid);
    printThreads(cur->nextThread, cur->tid);
}

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
            systemThread->nextThread = systemThread->firstContext = NULL;
    }
    // free(victim);  ??
}

thread rr_next() {
    return curThread ? curThread->nextThread : NULL;
}

tid_t lwp_create(lwpfun func, void *arg, size_t stackSize) {
    unsigned long *stack = malloc((stackSize + 1) * WORDSIZE);
    unsigned long *sp = stack + stackSize - 1;
    rfile state;
    thread threadSpace = malloc(sizeof(context));
    context newContext;

    if (currentScheduler == NULL) {
        currentScheduler = rrScheduler;
    } 
    if (curThread == NULL) {
        systemThread = &systemContext;
        curThread = systemThread;
    }

    sp = (char *)sp - ((unsigned long)sp % 16);

    *sp-- = (unsigned long) lwp_exit;
    *sp-- = (unsigned long) func;
    state.rsp = state.rbp = (unsigned long) sp;
    state.rdi = (unsigned long) arg;

    newContext = (context) {nextTid++, stack, stackSize, state, NULL, NULL,
            NULL, NULL};
    memcpy(threadSpace, &newContext, sizeof(context));
    threadSpace->state.fxsave = FPU_INIT;
    currentScheduler->admit(threadSpace);
    return threadSpace->tid;
}

void lwp_exit() {
    thread next;

    currentScheduler->remove(curThread);
    next = currentScheduler->next();

    if (next == NULL || curThread == next) { // used to have if next==NULL
        return lwp_stop();
    }
    SetSP(next->state.rsp);
    //printf("freeing %p and %p\n", curThread->stack, curThread);
    free(curThread->stack);
    free(curThread);
    curThread = next;
    load_context(&next->state);
    //switchContext(next);
    //exit(0);
}
    /* old stuff for lwp_exit after the if(next==null) stmt
    //curThread->lastThread->nextThread = next;
    //next->lastThread = curThread->lastThread;
    // need to check first/lastContext in scheduler?
    //free(cur->stack);
    //free(cur);
    */

tid_t lwp_gettid() {
    if (curThread == NULL) 
        return NO_THREAD;
    return curThread->tid;
}

void lwp_stop() {
    //curThread = systemThread;
    //load_context(&systemThread->state);
    systemThread->nextThread = curThread->nextThread;
    switchContext(systemThread);
}

scheduler lwp_get_scheduler() {
    return currentScheduler;
}

void lwp_set_scheduler(scheduler new) {
    currentScheduler = new;
}

void switchContext(thread to) {
    thread old = curThread;

    /* if to is NULL stay on same thread */
    if (!to)
        return;

    curThread = to;
    swap_rfiles(&(old->state), &(to->state));
}

void lwp_yield() {
    thread next = NULL;
    if (currentScheduler)
        next = currentScheduler->next();
    if (next)
        switchContext(next);
}

void lwp_start() {
    thread next = NULL;
    if (currentScheduler)
        next = currentScheduler->next();
    if (next)
        switchContext(next);
}

thread tid2thread(tid_t tid) {
    return tid2threadHelper(systemThread->firstContext, tid);
}

thread tid2threadHelper(thread cur, tid_t tid) {
    if (cur == NULL)
        return NULL;
    else if (cur->tid == tid)
        return cur;
    else
        return tid2threadHelper(cur->nextThread, tid);
}
