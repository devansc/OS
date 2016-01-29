#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lwp.h"
#include "schedulers.h"

#define MAXSNAKES  100
#define INITIALSTACK 2048

typedef void (*sigfun)(int signum);
static void indentnum(uintptr_t num);

int main(int argc, char *argv[]){
  long i;

  lwp_set_scheduler(AlwaysZero);

  printf("Launching LWPS\n");

  /* spawn a number of individual LWPs */
  for(i=1;i<=5;i++) {
    lwp_create((lwpfun)indentnum,(void*)i,INITIALSTACK);
  }

  lwp_start();                     /* returns when the last lwp exits */

  printf("Back from LWPS.\n");
  return 0;
}

static void indentnum(uintptr_t num) {
  /* print the number num num times, indented by 5*num spaces
   * Not terribly interesting, but it is instructive.
   */
  int howfar,i;

  howfar=(int)num;              /* interpret num as an integer */
  for(i=0;i<howfar;i++){
    printf("%*d\n",howfar*5,howfar);
    lwp_yield();                /* let another have a turn */
  }
  lwp_exit();                   /* bail when done.  This should
                                 * be unnecessary if the stack has
                                 * been properly prepared
                                 */
}

