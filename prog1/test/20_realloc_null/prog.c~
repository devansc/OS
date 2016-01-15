#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>


int main(int argc, char *argv[]){
  unsigned char *val;
  int i,err;

  printf("Allocating 8192 regions, size 0..8191...");
  err=0;
  for(i=0;i < 8192 && !err ;i++ ) {
    size_t size = i;

    val = realloc (NULL, size);
    if ( !val && size )
      err++;
    else {
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
