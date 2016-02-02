#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lwp.h"


#define nextThread lib_one
#define prevThread lib_two
#define WORDSIZE sizeof(unsigned long)

/* Globals */
scheduler currentScheduler = NULL; 
rfile systemState;
thread head = NULL; /* global pointer to head of queue */
thread tail = NULL; /* global pointer to tail of queue */
tid_t nextTid = 1;

extern scheduler rrScheduler;

/*
 * Creates a new lightweight process which executes the given function with 
 * the given argument. The new processes’s stack will be stacksize words.
 * lwp create() returns the tid of the new thread or -1 if the thread cannot
 * be created.
 */
tid_t lwp_create(lwpfun func, void *arg, size_t stackSize) {
    unsigned long *stack = malloc((stackSize + 1) * WORDSIZE);
    unsigned long *sp = stack + stackSize - 1;
    rfile state;
    thread threadSpace = malloc(sizeof(context));

    if (threadSpace == NULL)
        return -1;

    if (currentScheduler == NULL) {
        currentScheduler = rrScheduler;
    } 

    sp = (unsigned long *)((char *)sp - ((unsigned long)sp % 16));

    *sp-- = (unsigned long) lwp_exit;
    *sp-- = (unsigned long) func;
    state.rsp = state.rbp = (unsigned long) sp;
    state.rdi = (unsigned long) arg;

    threadSpace->tid = nextTid++;
    threadSpace->stack = stack;
    threadSpace->stacksize = stackSize;
    threadSpace->state = state;
    threadSpace->state.fxsave = FPU_INIT;

    if (head == NULL) {
        head = tail = threadSpace;
    } else {
        threadSpace->nextThread = head;
        threadSpace->prevThread = tail;
        head->prevThread = threadSpace;
        tail->nextThread = threadSpace;
        tail = threadSpace;
    }

    currentScheduler->admit(threadSpace);

    return threadSpace->tid;
}

/*
 * Terminates the current LWP and frees its resources. Gets the next thread
 * from the scheduler and switches to it or calls lwp_stop if there is none.
 */
void lwp_exit() {
    thread next;

    currentScheduler->remove(head);
    next = currentScheduler->next();

    if (next == NULL) { // || head == next) { 
        return lwp_stop();
    }
    SetSP(next->state.rsp);
    free(head->stack);
    head = next;
    load_context(&next->state);
}

/*
 * Returns the tid of the calling LWP thread or NO_THREAD if not called by 
 * a LWP. 
 */
tid_t lwp_gettid() {
    if (head == NULL) 
        return NO_THREAD;
    return head->tid;
}

/*
 * Stops the LWP system, restores the original stack pointer and returns to 
 * that context. 
 */
void lwp_stop() {
    swap_rfiles(&head->state, &systemState);
}

/* 
 * Gets the current scheduler. 
 */
scheduler lwp_get_scheduler() {
    return currentScheduler;
}

/* 
 * Sets the current scheduler to a new scheduler. Pulls out all threads 
 * contained in the current scheduler and puts them in the new one. 
 */
void lwp_set_scheduler(scheduler new) {
    thread next;
    
    if (new->init != NULL) {
        new->init();
    }
    while (currentScheduler && (next = currentScheduler->next())) {
        currentScheduler->remove(next);
        new->admit(next);
    }
    if (currentScheduler && currentScheduler->shutdown != NULL) {
        currentScheduler->shutdown();
    }
    currentScheduler = new;
}

/* 
 * Switches the context or thread to a new thread. Sets head to the new thread.
 */
void switchContext(thread to) {
    thread old = head;

    /* if to is NULL stay on same thread */
    if (!to)
        return;

    head = to;
    swap_rfiles(&(old->state), &(to->state));
}

/* 
 * Yields control to another LWP. Calls the scheduler to find out what thread
 * to yield to. Switches context to the new thread if exists, or the system 
 * thread if not. 
 */ 
void lwp_yield() {
    thread next = NULL;
    if (currentScheduler)
        next = currentScheduler->next();

    if (next)
        switchContext(next);
    else 
        swap_rfiles(&head->state, &systemState);
}

/*
 * Starts the LWP system. Saves the current context in systemState.
*/
void lwp_start() {
    thread next = NULL;
    if (currentScheduler)
        next = currentScheduler->next();
    if (next) {
        head = next;
        swap_rfiles(&systemState, &next->state);
    }
}

/* 
 * Returns the thread corresponding to the given tid or NULL if the tid is
 * invalid.
 */
thread tid2thread(tid_t tid) {
    if (head)
        return tid2threadHelper(head, tid, head->tid, 0);
    else return NULL;
}

/*
 * Recursive helper for tid2thread. Stops when has already seen the tid_t seen.
*/
thread tid2threadHelper(thread cur, tid_t tid, tid_t seen, int itr) {
    if (cur == NULL ||  (itr > 0 && cur->tid == seen)) {
        return NULL;
    } else if (cur->tid == tid) {
        return cur;
    } else {
        return tid2threadHelper(cur->nextThread, tid, seen, itr+1);
    }
}
