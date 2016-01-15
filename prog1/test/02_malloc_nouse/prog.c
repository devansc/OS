#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <lib.h>

int main(int argc, char *argv[]){
  void *val;
  val = malloc (1024);
  if ( val )
    printf("Calling malloc succeeded.\n");
  exit(0);
}
