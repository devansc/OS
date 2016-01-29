#ifndef SEMSH
#define SEMSH

#include <lwp.h>

extern scheduler Semaphores;
extern struct scheduler s_publish; /* for static initialization */


typedef struct semaphore *Semaphore;

Semaphore newsem(char *name, int num);
void up(Semaphore s);
void down(Semaphore s);
int rql();                      /* returns the runQueue length */

#endif
