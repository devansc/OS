#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>

#define SIZE (1<<24)

int main(int argc, char *argv[]){
  void *val;

  val= malloc (SIZE);
  if ( val )
    printf("Calling malloc succeeded.\n");
  else {
    printf("malloc() returned NULL.\n");
    exit(1);
  }

  fill(val,SIZE,0);
  printf("Successfully used the space.\n");

  if ( check(val,SIZE,0) )
    printf("Some values didn't match in region %p.\n",val);

  exit(0);
}
