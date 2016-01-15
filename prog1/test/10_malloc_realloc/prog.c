#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>


int main(int argc, char *argv[]){
  unsigned char *val;
  int size;

  size = 1028;
  val = malloc(size);
  fill(val,size,0);
  if ( !check(val,size,0) )
    printf("ok.\n");
  else
    printf("FAILED.\n");

  /* realloc */
  val = realloc(val,size*2);

  /* make sure original contents are the same */
  if ( !check(val,size,0) )
    printf("Contents ok after realloc.\n");
  else
    printf("Contents DIFFER after realloc.\n");

  /* try using new contents */
  fill(val,size*2,0);
  if ( !check(val,size,0) )
    printf("Fill of reallocated space ok.\n");
  else
    printf("Fill of reallocated space FAILED.\n");

  exit(0);
}
