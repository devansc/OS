#include "malloc.h"

int main(int argc, char **argv) {
  unsigned char *val;

  val = malloc (1024);
  if ( val )
    printf("Calling malloc succeeded.\n");
  else {
    printf("malloc() returned NULL.\n");
    exit(1);
  }

    return 0;
}
