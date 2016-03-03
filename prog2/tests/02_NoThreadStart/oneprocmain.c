/*
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "lwp.h"


int main(int argc, char *argv[]){
  printf("About to call lwp_start() with no threads...");
  lwp_start();
  printf("ok.\n");
  return 0;
}


