#include "malloc.h"
#include <stdio.h>
#include <unistd.h>

#define HEAPBLOCKSIZE 65536

extern void exit(int);

HeapPointer heap;

void init() {
    if (heap.start == NULL) {
        heap = newHeapPointer();
    }
}

HeapPointer newHeapPointer() {
    HeapPointer hp;
    hp.start = sbrk(HEAPBLOCKSIZE);
    if (hp.start == (void *) -1) {
        perror("sbrk (createHeapPointer)");
        exit(-1);
    }
    hp.currentLocation = hp.start;
    hp.availBytes = HEAPBLOCKSIZE;
    hp.next = NULL;
    return hp;
}

void * malloc(size_t size) {
    init();
    if (heap.availBytes < size) {
        // have to create another heap pointer
    }
    return (void *) NULL;
}



size_t padSize(size_t size) {
    size = size / 16;
    size = size + 1;
    return size * 16;
}
