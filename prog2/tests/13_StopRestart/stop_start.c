#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <lwp.h>

static void test(void *b)
{
   printf("Test\n");
   lwp_stop();
   printf("Test2\n");
   lwp_yield();
   printf("Test3\n");
   lwp_stop();
   printf("Test4\n");
   lwp_yield();
   printf("Test5\n");
   lwp_yield();
   printf("Test6\n");
   lwp_exit();
}


int main() {
  long i;
  printf ("Starting LWPs...\n");
  for(i=0;i<3;i++)
    lwp_create(test,(void*)i,8192);
  lwp_start();
  lwp_start();
  lwp_start();
  lwp_start();
  lwp_start();
  lwp_start();
  lwp_start();
  printf ("Done.\n");
  exit(0);
}
