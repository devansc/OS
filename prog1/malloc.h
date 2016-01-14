#include <stddef.h>
#include <stdint.h>

struct AllocUnit {
    size_t size;
    void * memLoc;
    struct AllocUnit * next;
    struct AllocUnit * last;
    int isFree;
}__attribute__((aligned (16)));

typedef struct AllocUnit AllocUnit;

void *malloc(size_t size);
AllocUnit *newAllocUnit(uintptr_t location, size_t size_block);
AllocUnit *getFreeAU(AllocUnit *cur, size_t sizeWanted);
uintptr_t moveHeapPointer(size_t numBytes);
void unfreeAU(AllocUnit *au, size_t size);
size_t padSize(size_t size);


void free(void *ptr);
void freePointer(AllocUnit *current, uintptr_t ptr);

void * calloc(size_t nmemb, size_t size);
void * realloc(void *ptr, size_t size);
void mergeAU(AllocUnit *au);
AllocUnit *reallocate(AllocUnit *au, size_t size);
AllocUnit * findAU(AllocUnit *cur, uintptr_t ptr);
AllocUnit *allocateNew(size_t size);
