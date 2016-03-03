#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>

#define BLOCK 23

#define ITERS 10000000

int main(int argc, char *argv[]){
  void *val;
  int i;
  char buff[1000];
  sprintf(buff, "%p", buff);

  printf("Starting looping...");
  for(i=0; i < ITERS; i++) {
    val=malloc(BLOCK);
    if ( val )
      val=realloc(val,0);
    else {
      printf("malloc() returned NULL\n");
      exit(1);
    }
  }

  printf("Well, we didn't crash.\n");
  exit(0);
}
