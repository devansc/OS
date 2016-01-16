#include "malloc.h"

int main(int argc, char **argv) {
  void *val;
  int i = 0;
  while(val) {
    val=malloc(1000000);
    i++;
  }
  printf("mallocd %d\n", i);
  perror("hit error");
  exit(0);
}
