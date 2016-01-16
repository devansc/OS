/*
 * Author: Devan Carlson (decarlso)
 *
 * This program is a memory allocater library. It implements the 4 stdlib 
 * functions malloc, calloc, realloc and free. 
 *
 */
#include "malloc.h"
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#define ALLOCBLOCKSIZE 65536
#define PADSIZE 16

extern int printf(const char *format, ...);
extern void exit(int);
extern char *getenv(const char *name);

/* Globals */
AllocUnit *startHeap; /* Start of the heap - first place to malloc */

/*
 * Initializes the startHeap if it's uninitialized - happens once per program.
 */
void init() {
    if (startHeap == NULL) {
        startHeap = newAllocUnit(moveHeapPointer(ALLOCBLOCKSIZE), 
                ALLOCBLOCKSIZE);
    }
}

/*
 * Returns true if DEBUG_MALLOC env variable is set.
 */
int debugMalloc() {
    if (getenv("DEBUG_MALLOC") != NULL) {
        return 1;      
    }
    return 0;
}

/*
 * Implementation of stdlib calloc.
 */
void * calloc(size_t nmemb, size_t size) {
    int paddedSize;
    AllocUnit *au;
    paddedSize  = padSize(nmemb * size);
    au = allocateNew(paddedSize);
    memset(au->memLoc, 0, paddedSize);
    if (debugMalloc()) {
        printf("MALLOC: calloc(%zu,%zu)     =>   (ptr=%p, size=%d)\n",nmemb,
                size,au->memLoc,au->size); 
    }
    return au->memLoc;
}

/*
 * Implementation of stdlib realloc.
 */
void * realloc(void *ptr, size_t size) {
    AllocUnit *au, *newAU;
    size_t paddedSize;

    if (size == 0) {
        free(ptr);
        return NULL;
    }
    paddedSize = padSize(size);
    init();

    au = findAU(startHeap, (uintptr_t) ptr);
    
    if (au == NULL || ptr == NULL) {
        return malloc(size);
    }

    newAU = reallocate(au, paddedSize);
    if (debugMalloc()) {
        printf("MALLOC: realloc(%p,%zu)    =>   (ptr=%p, size=%d)\n",ptr,
                size,newAU->memLoc,newAU->size); 
    }
    return newAU->memLoc;
}

/* 
 * Merges one AllocUnit with it's next AllocUnit. Must check if au->next
 * is free before calling.
 */
void mergeAU(AllocUnit *au) {
    AllocUnit *next = au->next;
    size_t increaseSize;

    if (next == NULL) return;  /* shouldn't happen */
    
    increaseSize = next->size + sizeof(AllocUnit);
    
    au->size += increaseSize;
    au->next = next->next;
    if (au->next != NULL) au->next->last = au;
}

/*
 * Attempts to reallocate an AllocUnit au. Attempts in place expansion, and 
 * if not, merges with neighboring free AllocUnits if possible. Then if that
 * isn't enough size, allocates a new AllocUnit with the correct size.
 */
AllocUnit *reallocate(AllocUnit *au, size_t size) {
    AllocUnit *newAU;
    size_t origSize = size;
    if (au->size >= size) {
        au->size = size;
        au->isFree = 0;
        return au;
    }

    /* Merges with next AU if free */
    if (au->next != NULL && au->next->isFree) {
        mergeAU(au);
    }
    /* Merges with last AU if free */
    if (au->last != NULL && au->last->isFree) {
        mergeAU(au->last);
    }

    if (au->last != NULL && au->last->isFree && au->last->size >= size) {
        newAU = au->last;
        unfreeAU(au->last, size);
        memmove(au->last->memLoc, au->memLoc, origSize);
        return newAU;
    } else if (au->size >= size) {
        unfreeAU(au, size);
        return au; 
    } else {
        newAU = allocateNew(size);
        memmove(newAU->memLoc, au->memLoc, origSize);
        return newAU;
    }
}

/* 
 * Finds an AllocUnit that contains location ptr in the Heap.
 */
AllocUnit * findAU(AllocUnit *cur, uintptr_t ptr) {
    uintptr_t loc = (uintptr_t) cur->memLoc;
    if (ptr < loc + cur->size && ptr >= loc) {
        return cur;
    } else if (cur->next != NULL) {
        return findAU(cur->next, ptr);
    } else {
        return NULL;
    }

}

/*
 * Implementation of stdlib malloc.
 */
void * malloc(size_t size) {
    AllocUnit *au = allocateNew(size);
    if (debugMalloc()) {
        printf("MALLOC: malloc(%zu)        =>   (ptr=%p, size=%d)\n", size,
                au->memLoc,au->size); 
    }
    return au->memLoc;
}

/*
 * Finds a free AllocUnit of the correct size, and returns it. Also creates 
 * the next free AllocUnit if the next AU is NULL with the available space.
 */
AllocUnit *allocateNew(size_t size) {
    AllocUnit *freeAU;
    size_t paddedSize = padSize(size);
    init();

    freeAU = getFreeAU(startHeap, paddedSize);

    unfreeAU(freeAU, paddedSize);

    return freeAU;
}

/*
 * Implementation of stdlib free.
 */
void free(void *ptr) {
    if (ptr == NULL) return;
    freePointer(startHeap, (uintptr_t) ptr);
    if (debugMalloc()) {
        printf("MALLOC: free(%p)\n",ptr);
    }
}

/*
 * Finds the AU pointed to by ptr, and frees it. Attempts to expand with 
 * neighboring free AU's. 
 */
void freePointer(AllocUnit *current, uintptr_t ptr) {
    uintptr_t loc = (uintptr_t) current->memLoc;
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
        return; /*couldnt find pointer just return cleanly */
    }
}

/*
 * Unfree's an AU and sets it size to 'size'. If au doesn't have a next, then
 * it creates a new AU with the available free space. 
 */
void unfreeAU(AllocUnit *au, size_t size) {
    long freeSpace = au->size - size - sizeof(AllocUnit);
    uintptr_t location;
    AllocUnit *nextAU;
    int needsCopy = 0;

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
    }

    if (nextAU != NULL) {
        nextAU->last = au;
        au->next = nextAU;
    }
}

/*
 * Finds a free AU from the heap. If there are none available, creates more 
 * heap space and returns a new AU. 
 */
AllocUnit *getFreeAU(AllocUnit *cur, size_t sizeWanted) {
    AllocUnit *newAU;

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
        newAU = newAllocUnit(moveHeapPointer(sizeWanted), 
        sizeWanted + sizeof(AllocUnit));
        cur->next = newAU;
        newAU->last = cur;
        return newAU;
    }
}

/*
 * Moves the heap pointer using sbrk in increments of PADSIZE.
 */
uintptr_t moveHeapPointer(size_t numBytes) {
    uintptr_t location;
    uintptr_t oldLoc;

    oldLoc = sbrk(0);
    if (oldLoc % PADSIZE) sbrk(PADSIZE-(oldLoc % PADSIZE));
    
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

    newAU.size = size_block;
    newAU.memLoc = (void *)memLocation;
    newAU.next = newAU.last = NULL;
    newAU.isFree = 1;

    memmove((void *)location, &newAU, sizeof(AllocUnit));
    return (AllocUnit *) location;
}

/*
 * Pads (aligns) a size to PADSIZE bytes.
 */
size_t padSize(size_t size) {
    if (size % PADSIZE == 0) return size;
    size = size / PADSIZE;
    size = size + 1;
    return size * PADSIZE;
}
