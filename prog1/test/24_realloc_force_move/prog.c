#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>

/* allocate three buffers, then grow the middle one so a move is forced */

int main(int argc, char *argv[]){
  unsigned char *one, *two, *three,*new;

  one = getbuff(32);          /* allocate and fill buffers */
  two = getbuff(32);
  three = getbuff(32);

  new = realloc(two,4096);
  if ( new == NULL ) {
    printf("Realloc returned NULL.\n");
  }
  if ( new == two ) {
    printf("Realloc did not move the buffer as expected.\n");
  }

  if ( check(new,32,0) )
    printf("Some values were not properly copied.\n");
  if ( check(one,32,0) )
    printf("Values in buffer one were disturbed\n");
  if ( check(three,32,0) )
    printf("Values in buffer three were disturbed\n");

  /* use the space to be sure */
  fill(new,4096,0);

  exit(0);
}
