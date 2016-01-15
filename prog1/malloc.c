#include "malloc.h"
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define ALLOCBLOCKSIZE 65536
#define PADSIZE 16
#define DEBUG 0

extern int printf(const char *format, ...);
extern void exit(int);
extern char *getenv(const char *name);

AllocUnit *startHeap;

void init() {
    if (startHeap == NULL) {
        if (DEBUG)
            printf("initialized startHeap\n");
        startHeap = newAllocUnit(moveHeapPointer(ALLOCBLOCKSIZE), 
                ALLOCBLOCKSIZE);
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
        printf("   %p-%p, %zu, %s\n", cur->memLoc, cur->memLoc+cur->size, cur->size, 
            cur->isFree ? "free" : "used");
        cur = cur->next;
    }
}

void * calloc(size_t nmemb, size_t size) {
    int paddedSize;
    AllocUnit *au;
    paddedSize  = padSize(nmemb * size);
    au = allocateNew(paddedSize);
    if (DEBUG)
        printf("calloc\n");
    memset(au->memLoc, 0, paddedSize);
    if (debugMalloc()) {
        printf("MALLOC: calloc(%zu,%zu)     =>   (ptr=%p, size=%d)\n",nmemb,
                size,au->memLoc,au->size); 
    }
    return au->memLoc;
}

void * realloc(void *ptr, size_t size) {
    AllocUnit *au, *newAU;
    size_t paddedSize;

    if (size == 0 && ptr != NULL) {
        free(ptr);
    }
    paddedSize = padSize(size);
    if (DEBUG)
        printf("realloc %p - %zu", ptr, size);
    if (DEBUG)
        printHeap(startHeap);
    init();

    au = findAU(startHeap, (uintptr_t) ptr);
    
    if (au == NULL || ptr == NULL) {
        return malloc(size);
    }

    newAU = reallocate(au, paddedSize);
    if (DEBUG)
        printf("done reallocing\n");
    if (DEBUG)
        printHeap(startHeap);
    if (debugMalloc()) {
        printf("MALLOC: realloc(%p,%zu)    =>   (ptr=%p, size=%d)\n",ptr,
                size,newAU->memLoc,newAU->size); 
    }
    return newAU->memLoc;
}

void mergeAU(AllocUnit *au) {
    AllocUnit *next = au->next;
    size_t increaseSize;
    if (DEBUG)
        printf("mergeAU\n");

    if (next == NULL) return;  /* shouldn't happen */
    
    increaseSize = next->size + sizeof(AllocUnit);
    
    au->size += increaseSize;
    au->next = next->next;
    if (au->next != NULL) au->next->last = au;
}

AllocUnit *reallocate(AllocUnit *au, size_t size) {
    AllocUnit *newAU;
    size_t origSize = size;
    if (DEBUG)
        printf("reallocate\n");
    if (au->size >= size) {
        au->size = size;
        au->isFree = 0;
        return au; /*all we have to do here?*/
    }
    if (au->next != NULL && au->next->isFree) {
        mergeAU(au);
    }
    if (au->last != NULL && au->last->isFree) {
        mergeAU(au->last);
    }
    if (au->last != NULL && au->last->isFree && au->last->size >= size) {
        /* au->last->size = size; */
        newAU = au->last;
        unfreeAU(au->last, size);
        if (DEBUG)
            printf("copying from %p to %p (%zu)\n", au->memLoc,
                 au->last->memLoc, origSize);
        memmove(au->last->memLoc, au->memLoc, origSize);
        if (DEBUG)
            printf("returning AU %p\n", au->last);
        return newAU;
    } else if (au->size >= size) {
        /*au->size = size;*/
        unfreeAU(au, size);
        return au; 
    } else {
        newAU = allocateNew(size);
        if (DEBUG)
            printf("copying from %p to %p (%zu)\n", au, newAU->memLoc, 
                    origSize);
        if (DEBUG)
            printf("copying memory... ");
        if (DEBUG)
            printf("copying memory... ");
        memmove(newAU->memLoc, au->memLoc, origSize);
        if (DEBUG)
            printf("copied\n");
        return newAU;
    }
}

AllocUnit * findAU(AllocUnit *cur, uintptr_t ptr) {
    uintptr_t loc = (uintptr_t) cur->memLoc;
    if (DEBUG)
        /*printf("findAU\n");*/
    if (ptr < loc + cur->size && ptr >= loc) {
        return cur;
    } else if (cur->next != NULL) {
        return findAU(cur->next, ptr);
    } else {
        return NULL;
    }

}

void * malloc(size_t size) {
    if (DEBUG)
        printf("malloc\n");
    if (DEBUG)
        printf("before malloc - ");
    if (DEBUG)
        printHeap(startHeap);
    AllocUnit *au = allocateNew(size);
    if (debugMalloc()) {
        printf("MALLOC: malloc(%zu)        =>   (ptr=%p, size=%d)\n", size,
                au->memLoc,au->size); 
        if (DEBUG)
            printHeap(startHeap);
    }
    return au->memLoc;
}

AllocUnit *allocateNew(size_t size) {
    AllocUnit *freeAU;
    size_t paddedSize = padSize(size);
    if (DEBUG)
        printf("allocateNew\n");
    init();

    freeAU = getFreeAU(startHeap, paddedSize);

    unfreeAU(freeAU, paddedSize);

    if (DEBUG)
        printf("alloc'd at %p\n", freeAU->memLoc);
    return freeAU;
}

void free(void *ptr) {
    if (DEBUG)
        printf("free\n");
    if (DEBUG)
        printf("before free - ");
    if (DEBUG)
        printHeap(startHeap);
    if (ptr == NULL) return;
    freePointer(startHeap, (uintptr_t) ptr);
    if (debugMalloc()) {
        printf("MALLOC: free(%p)\n",ptr);
        if (DEBUG)
            printHeap(startHeap);
    }
}

void freePointer(AllocUnit *current, uintptr_t ptr) {
    uintptr_t loc = (uintptr_t) current->memLoc;
    if (DEBUG)
        printf("freePointer\n");
    if (DEBUG)
        printf("trying to free %p, current %p-%p\n", ptr, loc, 
        loc+current->size);
    if (ptr < loc + current->size && ptr >= loc) {
        current->isFree = 1;
        /* check for neighboring free space and merge */
        if (current->next != NULL && current->next->isFree) 
            mergeAU(current);
        if (current->last != NULL && current->last->isFree) 
            mergeAU(current->last);
    } else if (current->next != NULL) {
        freePointer(current->next, ptr);
    } else {
        if (DEBUG)
            printf("free unkown space %X\n", ptr);
        if (DEBUG)
            printHeap(startHeap);
        return; /*couldnt find pointer just return cleanly */
    }
}

void unfreeAU(AllocUnit *au, size_t size) {
    long freeSpace = au->size - size - sizeof(AllocUnit);
    uintptr_t location;
    AllocUnit *nextAU;
    int needsCopy = 0;
    if (DEBUG)
        printf("unfreeAU\n");

    location = (uintptr_t)au->memLoc + size;
    au->size = size;
    au->isFree = 0;
    if (freeSpace > 0 && freeSpace >= sizeof(AllocUnit) + PADSIZE 
            && au->next == NULL) {
        nextAU = newAllocUnit(location, freeSpace);
    } else if (au->next != NULL) {
        nextAU = au->next;
    } else if (au->next == NULL) {
        nextAU = newAllocUnit(moveHeapPointer(ALLOCBLOCKSIZE), ALLOCBLOCKSIZE);
    } else{
        nextAU = NULL;
        if (DEBUG)
            printf("not changing next\n");
    }

    if (nextAU != NULL) {
        /* nextAU->next = au->next; WTF was i thinking right here */

        nextAU->last = au;
        au->next = nextAU;
    }
}

AllocUnit *getFreeAU(AllocUnit *cur, size_t sizeWanted) {
    AllocUnit *newAU;
    if (DEBUG)
        /*printf("getFreeAU\n");*/

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
        newAU = newAllocUnit(moveHeapPointer(sizeWanted + sizeof(AllocUnit)), 
        sizeWanted + sizeof(AllocUnit));
        cur->next = newAU;
        newAU->last = cur;
        if (DEBUG)
            printf("After bigMalloc -- ");
        if (DEBUG)
            printHeap(startHeap);
        return newAU;
    }
}

uintptr_t moveHeapPointer(size_t numBytes) {
    uintptr_t location;
    uintptr_t oldLoc;
    if (DEBUG)
        printf("moveHeapPointer from ");

    oldLoc = sbrk(0);
    if (oldLoc % 16) sbrk(16-(oldLoc % 16));
    
    location = sbrk(numBytes + sizeof(AllocUnit));
    if (location == -1) {
        perror("sbrk");
        exit(-1); /* TODO return null somehow ... */
    }
    if (DEBUG)
        printf("%p -> %p\n", oldLoc, location + numBytes + sizeof(AllocUnit));
    return location;
}

/*
 * Creates a new, free AllocUnit at location, with size size_block
 */
AllocUnit *newAllocUnit(uintptr_t location, size_t size_block) {
    AllocUnit newAU;
    uintptr_t memLocation = location + sizeof(AllocUnit);
    if (DEBUG)
        printf("newAllocUnit %p\n", memLocation);

    newAU.size = size_block;
    newAU.memLoc = (void *)memLocation;
    newAU.next = newAU.last = NULL;
    newAU.isFree = 1;

    memmove((void *)location, &newAU, sizeof(AllocUnit));
    return (AllocUnit *) location;
}

size_t padSize(size_t size) {
    if (DEBUG)
        printf("padSize\n");
    /*if (size % 16 == 0) return size;*/
    size = size / PADSIZE;
    size = size + 1;
    return size * PADSIZE;
}
