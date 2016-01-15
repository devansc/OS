#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>


#define SIZE 8192
int main(int argc, char *argv[]){
  unsigned char *val;
  int size;
  int i,j,err;

  /* Allocate a region, then expand it 8181 times, with malloc() interspersed
   */
  printf("Allocating %d regions, size 1..%d...", SIZE, SIZE);
  err=0;

  size = 0;
  for(i=1;i < SIZE && !err ;i++ ) {
    size++;
    val = calloc(size,1);
    if ( !val && size )
      err++;
    else {
      for (j=0; i<size; j++)
        if ( val[j] )
          err++;

      fill(val,size,i);
      if ( check(val,size,i) )
        err++;
    }
  }
  if ( !err )
    printf("ok.\n");
  else
    printf("FAILED.\n");
  exit(0);
}
