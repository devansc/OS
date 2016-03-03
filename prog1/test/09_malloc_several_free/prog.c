#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>


#define HOWMANY 5
int main(int argc, char *argv[]){
  unsigned char *val[HOWMANY];
  int size[HOWMANY];
  int i;

  for(i=0;i<HOWMANY;i++ ) {
    size[i] = (i+1) * 500;  /* to avoid freeing NULL */

    printf("Allocating a region of size %d...", (int)size[i]);
    val[i] = malloc (size[i]);
    if ( !val[i] && size[i] )
      printf("FAILED.\n");
    else {
      fill(val[i],size[i],i);
      if ( !check(val[i],size[i],i) )
        printf("ok.\n");
      else
        printf("FAILED.\n");
    }
  }

  for(i=0;i<HOWMANY;i++ ) {
    int j;
    free(val[i]);
    /* check the rest...*/
    printf("Free ok. Checking remaining regions...(");
    for(j=i+1;j<HOWMANY;j++) {
      if ( !check(val[j],size[i],j) )
        printf("ok.");
      else
        printf("FAILED.");
    }
    printf(")\n");
  }

  exit(0);
}
