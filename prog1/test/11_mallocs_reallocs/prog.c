#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>

#define SIZE 8192



void errfun(int line) {
  return;
}

int main(int argc, char *argv[]){
  unsigned char *val, *block[SIZE];
  int size;
  int i,err;

  /* Allocate a region, then expand it 8181 times, with malloc() interspersed
   */
  printf("Allocating %d regions, size 1..%d...", SIZE, SIZE);
  err=0;

  block[0]  = malloc (1);
  *block[0]  = 'Q';
  val  = malloc (1);
  fill(val,1,0);
  size = 1;

  for(i=1;i < 8192 && !err ;i++ ) {
    size++;
    val = realloc(val,size);
    if ( !val && size ) {
      errfun(__LINE__);
      printf ("...realloc(%d) returned NULL...", size);
      err++;
    }
    else {
      if ( check(val,size-1,0) ) {
        printf ("...contents not properly copied...");
        errfun(__LINE__);
        err++;
      }
      fill(val,size,0);
      if ( check(val,size,0) ) {
        printf ("...filled val not found...");
        errfun(__LINE__);
        err++;
      }
    }
    block[i] = malloc(1);
    if ( !block[i] )
      err++;
    else {                      /* check that the blocks are unhurt */
      int j;
      *block[i]='Q';
      for(j=0; j<i && !err ; j++)
        if ( *block[j] != 'Q' ) {
          printf("...canary missing (buffov)...");
          errfun(__LINE__);
          err++;
        }
    }

  }
  if ( !err )
    printf("ok.\n");
  else
    printf("FAILED.\n");
  exit(0);
}
