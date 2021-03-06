#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>


int main(int argc, char *argv[]){
  unsigned char *val,*new;

  val = malloc (1024);
  if ( val )
    printf("Calling malloc succeeded.\n");
  else {
    printf("malloc() returned NULL.\n");
    exit(1);
  }

  fill(val,1024,0);
  printf("Successfully used the space.\n");

  if ( check(val,1024,0) )
    printf("Some values didn't match in region %p.\n",val);

  new = realloc(val,2048);
  if ( new != val ) {
    printf("Realloc unexpectedly moved the buffer old %p != new %p.\n",val,new);
  }

  if ( check(new,1024,0) )
    printf("Some values were not properly copied.\n");

  exit(0);
}
