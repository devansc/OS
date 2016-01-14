#include "malloc.h"

int main(int argc, char **argv) {
    char *a = (char*) malloc(6), *b, *c;
    strcpy(a, "hello");
    printf("a at %p --> %s\n", a, a);
    b = malloc(7);
    strcpy(b, "whasup");
    printf("b at %p --> %s\n", b, b);

    a = realloc(a, 200);
    strcpy(a, "whats up now im 200");
    printf("a at %p --> %s\n", a, a);

    return 0;
}
