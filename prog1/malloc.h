#include <stddef.h>

struct HeapPointer {
    void * start;
    void * currentLocation;
    size_t availBytes;
    struct HeapPointer * next;
}__attribute__((aligned (16)));

struct AllocUnit {
    int size;
    struct AllocUnit * next;
    int isFree;
}__attribute__((aligned (16)));

typedef struct HeapPointer HeapPointer;
typedef struct AllocUnit AllocUnit;

void *malloc(size_t size);
HeapPointer newHeapPointer();
