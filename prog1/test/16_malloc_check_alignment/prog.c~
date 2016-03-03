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

    val = malloc (size);
    if ( !val && size ) {
      fprintf(stderr,"malloc(%d) failed: 0x%p\n",
              i,val);
      err++;
    } else {
      /* check to see if the returned address lines up mod 16 */
      if ( (long)val & (long)0xf ) {
        fprintf(stderr,"malloc(%d) returned unaligned pointer: 0x%p\n",
                i,val);
        err++;
        break;
      }
    }
  }
  if ( !err )
    printf("ok.\n");
  else
    printf("FAILED.\n");
  exit(0);
}
