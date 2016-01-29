
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lwp.h"

#define INITIALSTACK 2048
#define ROUNDS 1000

typedef void (*sigfun)(int signum);
static void indentnum(uintptr_t num);

int comp_tids(const void *ap, const void *bp) {
  tid_t a=*(tid_t*)ap;
  tid_t b=*(tid_t*)bp;
  return b-a;
}



int main(int argc, char *argv[]){
  long i;
  tid_t threads[ROUNDS];

  printf("Creating LWPS\n");

  /* spawn a number of individual LWPs */
  for(i=0;i<ROUNDS;i++) {
    threads[i]=lwp_create((lwpfun)indentnum,(void*)i,INITIALSTACK);
  }

  /* sort the tids */
  qsort(threads,ROUNDS,sizeof(tid_t),comp_tids);

  /* look for duplicates */
  for(i=0;i<ROUNDS-1;i++) {
    if ( threads[i] == threads[i+1] ) {
      fprintf(stderr, "Duplicate thread ID found.\n");
      exit(1);
    }
  }
  return 0;
}

static void indentnum(uintptr_t num) {
  /* print the number num num times, indented by 5*num spaces
   * Not terribly interesting, but it is instructive.
   */
  int howfar,i;

  howfar=(int)num;              /* interpret num as an integer */
  for(i=0;i<ROUNDS;i++){
    printf("%*d\n",howfar*5,howfar);
    if ( num == 5 && i == 2 ) { /* end of third round */
      printf("Setting the scheduler.\n");
    }

    lwp_yield();                /* let another have a turn */
  }
  lwp_exit();                   /* bail when done.  This should
                                 * be unnecessary if the stack has
                                 * been properly prepared
                                 */
}

