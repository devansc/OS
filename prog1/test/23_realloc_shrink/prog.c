#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>

/* allocate three buffers, then shrink the middle one.  It is not expected
 * to move */

int main(int argc, char *argv[]){
  unsigned char *one, *two, *three,*new;

  one = getbuff(2048);          /* allocate and fill buffers */
  two = getbuff(2048);
  three = getbuff(2048);

  new = realloc(two,1024);
  if ( new != two ) {
    printf("Realloc unexpectedly moved the buffer old %p != new %p.\n",two,new);
  }

  if ( check(new,1024,0) )
    printf("Some values were not properly copied.\n");
  if ( check(one,2048,0) )
    printf("Values in buffer one were disturbed\n");
  if ( check(three,2048,0) )
    printf("Values in buffer three were disturbed\n");

  exit(0);
}
