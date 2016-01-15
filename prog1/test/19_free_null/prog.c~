#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>

#define SIZE 2431

#define HOWMANY 50
int main(int argc, char *argv[]){
  unsigned char *val;
  int i;

  for(i=0;i<HOWMANY;i++ ) {
    free(NULL);
  }
  if ( (val= malloc(SIZE)) == NULL ) {
    printf("malloc() returned NULL.\n");
    exit(1);
  }

  fill(val,SIZE,0);
  printf("Successfully used the space.\n");

  if ( check(val,SIZE,0) )
    printf("Some values didn't match in region %p.\n",val);

  exit(0);
}
