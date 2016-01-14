#include "malloc.h"
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define ALLOCBLOCKSIZE 256
#define PADSIZE 16

extern int printf(const char *format, ...);
extern void *memcpy(void *dest, const void *src, size_t n);
extern void exit(int);
extern char *getenv(const char *name);

AllocUnit *startHeap;

void init() {
    if (startHeap == NULL) {
        printf("initialized startHeap\n");
        startHeap = newAllocUnit(moveHeapPointer(ALLOCBLOCKSIZE), ALLOCBLOCKSIZE);
    }
}

int debugMalloc() {
    if (getenv("DEBUG_MALLOC") != NULL) {
        return 1;      
    }
    return 0;
}

void printHeap(AllocUnit *cur) {
    printf("HEAP:\n");
    while (cur != NULL) {
        printf("   %p, %zu, %s\n", cur->memLoc, cur->size, cur->isFree ? "free" : "used");
        cur = cur->next;
    }
}

void * calloc(size_t nmemb, size_t size) {
    AllocUnit *au = allocateNew(size);
    printf("calloc\n");
    memset(au->memLoc, 0, size);
    if (debugMalloc()) {
        printf("MALLOC: calloc(%zu,%zu)     =>   (ptr=%p, size=%d)\n",nmemb,
                size,au->memLoc,au->size); 
    }
    return au->memLoc;
}

void * realloc(void *ptr, size_t size) {
    AllocUnit *au, *newAU;
    size_t paddedSize = padSize(size);
    printf("realloc\n");
    init();

    au = findAU(startHeap, (uintptr_t) ptr);
    
    if (ptr == NULL) {
        return malloc(size);
    }

    newAU = reallocate(au, paddedSize);
    if (debugMalloc()) {
        printf("MALLOC: realloc(%p,%zu)    =>   (ptr=%p, size=%d)\n",ptr,
                size,newAU->memLoc,newAU->size); 
    }
    return newAU->memLoc;
}

void mergeAU(AllocUnit *au) {
    AllocUnit *next = au->next;
    size_t increaseSize;
    printf("mergeAU\n");

    if (next == NULL) return;  /* shouldn't happen */
    
    increaseSize = next->size;
    
    au->size += increaseSize;
    au->next = next->next;
}

AllocUnit *reallocate(AllocUnit *au, size_t size) {
    printf("reallocate\n");
    if (au->size >= size) {
        au->size = size;
        return au; /*all we have to do here?*/
    }
    if (au->next != NULL && au->next->isFree) {
        mergeAU(au);
    }
    if (au->last != NULL && au->last->isFree) {
        mergeAU(au->last);
    }
    if (au->last != NULL && au->last->isFree && au->last->size >= size) {
        au->last->size = size;
        return au->last;
    } else if (au->size >= size) {
        au->size = size;
        return au; 
    } else {
        return allocateNew(size);
    }
}

AllocUnit * findAU(AllocUnit *cur, uintptr_t ptr) {
    uintptr_t loc = (uintptr_t) cur->memLoc;
    printf("findAU\n");
    if (ptr < loc + cur->size && ptr >= loc) {
        return cur;
    } else if (cur->next != NULL) {
        return findAU(cur->next, ptr);
    } else {
        return NULL;
    }

}

void * malloc(size_t size) {
    printf("malloc\n");
    printf("before malloc - ");
    printHeap(startHeap);
    AllocUnit *au = allocateNew(size);
    if (debugMalloc()) {
        printf("MALLOC: malloc(%zu)        =>   (ptr=%p, size=%d)\n", size,
                au->memLoc,au->size); 
        printHeap(startHeap);
    }
    return au->memLoc;
}

AllocUnit *allocateNew(size_t size) {
    AllocUnit *freeAU;
    size_t paddedSize = padSize(size);
    printf("allocateNew\n");
    init();

    freeAU = getFreeAU(startHeap, paddedSize);
    printf("after getFree\n");
    printHeap(startHeap);

    unfreeAU(freeAU, paddedSize);
    printf("after unfree\n");
    printHeap(startHeap);

    printf("alloc'd at %p\n", freeAU->memLoc);
    return freeAU;
}

void free(void *ptr) {
    printf("free\n");
    printf("before free - ");
    printHeap(startHeap);
    if (ptr == NULL) return;
    freePointer(startHeap, (uintptr_t) ptr);
    if (debugMalloc()) {
        printf("MALLOC: free(%p)\n",ptr);
        printHeap(startHeap);
    }
}

void freePointer(AllocUnit *current, uintptr_t ptr) {
    uintptr_t loc = (uintptr_t) current->memLoc;
    printf("freePointer\n");
     printf("trying to free %p, current %p-%p\n", ptr, loc, loc+current->size);
    if (ptr < loc + current->size && ptr >= loc) {
        current->isFree = 1;
        /* check for neighboring free space and merge */
        if (current->next != NULL) mergeAU(current);
        if (current->last != NULL) mergeAU(current->last);
    } else if (current->next != NULL) {
        freePointer(current->next, ptr);
    } else {
        printf("free unkown space %X\n", ptr);
        printHeap(startHeap);
        return; /*couldnt find pointer just return cleanly */
    }
}

void unfreeAU(AllocUnit *au, size_t size) {
    size_t freeSpace = au->size - size;
    uintptr_t location;
    AllocUnit *nextAU;
    printf("unfreeAU\n");

    location = (uintptr_t)au->memLoc + size;
    au->size = size;
    au->isFree = 0;
    if (freeSpace >= sizeof(AllocUnit) + PADSIZE) {
        nextAU = newAllocUnit(location, freeSpace);
    } else {
        nextAU = NULL;
        printf("not changing next\n");
    }

    if (nextAU != NULL) {
        nextAU->next = au->next;
        nextAU->last = au;
        au->next = nextAU;
    }
}

AllocUnit *getFreeAU(AllocUnit *cur, size_t sizeWanted) {
    AllocUnit *newAU;
    int bigMalloc;
    printf("getFreeAU\n");

    if (cur->size >= sizeWanted && cur->isFree) {
        return cur;
    } else if (cur->next != NULL) {
        return getFreeAU(cur->next, sizeWanted);
    } else if (sizeWanted <= ALLOCBLOCKSIZE - sizeof(AllocUnit)) { 
        /* if cur->next is NULL we know cur is the top of the heap */
        newAU = newAllocUnit(moveHeapPointer(ALLOCBLOCKSIZE), ALLOCBLOCKSIZE);
        cur->next = newAU;
        newAU->last = cur;
        return newAU;
    } else {
        bigMalloc = sizeWanted + sizeof(AllocUnit);
        newAU = newAllocUnit(moveHeapPointer(bigMalloc), bigMalloc);
        cur->next = newAU;
        newAU->last = cur;
        printf("After bigMalloc -- ");
        printHeap(startHeap);
        return newAU;
    }
}

uintptr_t moveHeapPointer(size_t numBytes) {
    uintptr_t location;
    uintptr_t oldLoc;
    printf("moveHeapPointer\n");

    oldLoc = sbrk(0);
    if (oldLoc % 16) sbrk(16-(oldLoc % 16));
    
    location = sbrk(numBytes + sizeof(AllocUnit));
    if (location == -1) {
        perror("sbrk");
        exit(-1); /* TODO return null somehow ... */
    }
    return location;
}

/*
 * Creates a new, free AllocUnit at location, with size size_block
 */
AllocUnit *newAllocUnit(uintptr_t location, size_t size_block) {
    AllocUnit newAU;
    uintptr_t memLocation = location + sizeof(AllocUnit);
    printf("newAllocUnit\n");

    newAU.size = size_block;
    newAU.memLoc = (void *)memLocation;
    newAU.next = newAU.last = NULL;
    newAU.isFree = 1;

    memcpy((void *)location, &newAU, sizeof(AllocUnit));
    return (AllocUnit *) location;
}

size_t padSize(size_t size) {
    printf("padSize\n");
    /*if (size % 16 == 0) return size;*/
    size = size / PADSIZE;
    size = size + 1;
    return size * PADSIZE;
}
