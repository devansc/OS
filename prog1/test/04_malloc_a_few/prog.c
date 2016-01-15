#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>


int main(int argc, char *argv[]){
  unsigned char *val;
  int i;

  for(i=0;i<5;i++ ) {
    size_t size = i * 500;

    printf("Allocating a region of size %d...", (int)size);
    val = malloc (size);
    if ( !val && size )
      printf("FAILED.\n");
    else {
      fill(val,size,i);
      if ( !check(val,size,i) )
        printf("ok.\n");
      else
        printf("FAILED.\n");
    }
  }
  exit(0);
}
