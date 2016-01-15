#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>

int main(int argc, char *argv[]){
  unsigned char *val;
  int i, err;

  val = calloc (1,1024);
  if ( val )
    printf("Calling calloc succeeded.\n");
  else {
    printf("malloc() returned NULL.\n");
    exit(1);
  }

  err = 0;

  for (i=0; i<1024; i++)
    if ( val[i] )
      err++;

  if ( err )
    printf("Something wasn't cleared\n");

  fill(val,1024,0);
  printf("Successfully used the space.\n");

  if ( check(val,1024,0) )
    printf("Some values didn't match in region %p.\n",val);

  exit(0);
}
