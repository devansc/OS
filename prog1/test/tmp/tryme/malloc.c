#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

extern void *__sbrk(intptr_t);
void *sbrk(intptr_t size) {
  return __sbrk(size);
}

/*
 * Vocabulary:
 *    CHUNK: how much to move the break each time
 *    MIN_BLOCK: the smallest allowable allocation unit.  (really
 *               MIN_BLOCK + header_size
 */

#ifndef CHUNK
#define CHUNK (1<<16)
#endif

#define MIN_BLOCK 16

/* the number of hex digits in a pointer or long */
#define PTRWID ((int)(2*sizeof(void*)))


typedef struct mheader *mheader;
struct mheader {
  void *  base;                 /* base of allocated segment */
  size_t  size;                 /* size in bytes */
  int     free;                 /* is this free? */
  mheader prev;                 /* pointer to next record */
  mheader next;                 /* pointer to previous record */
};

static int debug_malloc=0;        /* flag for verbosity */

/* two useful globals: the head and the tail of the heap list */
static mheader memory=NULL;     /* the hunk o' memory we're allocating */
static mheader memend=NULL;     /* the last chunk in memory            */

/* one useful constant: size of a header, rounded up to
 * preserve alignment.
 */
#define HEADER_SIZE (MIN_BLOCK * (sizeof(struct mheader)/MIN_BLOCK + \
                                  ((sizeof(struct mheader)%MIN_BLOCK)?1:0)))


/* useful prototypes */
static size_t lmgt(size_t factor, size_t size);
static mheader find_block(mheader list, size_t size);
static mheader find_block_containing(mheader list, void *ptr);
static mheader expand_break(size_t hint);
static int malloc_init(void);
static void split_block(mheader block, size_t size);
static void merge_blocks(mheader one, mheader two);
static void internal_free(void *ptr);
static void *internal_malloc(size_t size);
void pm();                      /* debugging dump memlist */
static int logmsg(int level, FILE *stream, const char *format, ...); /* log */

static mheader find_block(mheader list, size_t size) {
  /* find a free hunk of size size or bigger in the list */
  mheader block;
  for(block=list; block && ( !block->free || block->size < size ) ;
      block=block->next)
    /* search the list */;

  return block;
}

static mheader find_block_containing(mheader list, void *ptr) {
  /* find the block containing the given pointer */
  while ( list &&
          ( (ptr < list->base) || ((list->base+list->size) <= ptr) ) )
    list = list->next;
  /* this'll be the block, or NULL if not found */
  return list;
}

static mheader expand_break(size_t hint) {
  /* expand the heap and return a pointer to header containing the new
   * space (or NULL on failure).
   * If hint is zero, requests CHUNK bytes.  If non-zero, requests the
   * smallest number of CHUNKs with enough space to hold hint.
   */
  void *brk, *rvalue;
  size_t ask, howmuch;
  int extra;

  if ( hint )  /* this is what we want */
    howmuch = lmgt(CHUNK,hint);
  else
    howmuch = CHUNK;

  ask = howmuch;


  logmsg(1,stderr,"%s: trying to add %u bytes to break...", __FUNCTION__,
            (unsigned) ask);

  if ( !memory ) {              /* This is the first time */
    brk = sbrk(0);              /* find the current break */
    if ( brk == (void*)-1) {
      logmsg(1,stderr,"FAILED.\n");
      return NULL;
    }

    /* round up to nearest aligned location:  This will be the base
     * of the new memory.  Make sure the heap is aligned, and that
     * the ask contains a full aligned CHUNK (or howmuch, if bigger)
     */
    extra = (uintptr_t)brk % MIN_BLOCK;
    if ( extra ) {
      ask += MIN_BLOCK - extra;
      brk += MIN_BLOCK - extra;
    }
    memory = brk;             /* this is now the head */

    /* move the break */
    brk = sbrk(ask);
    if ( brk == (void*)-1) {
      memory = NULL;
      perror(__FUNCTION__);
      return NULL;
    }
    /* it worked, hook it up. */
    memory->base = ((void *)memory) + HEADER_SIZE;
    memory->size = howmuch - HEADER_SIZE;
    memory->free = TRUE;
    memory->prev = NULL;
    memory->next = NULL;
    memend = memory;
    rvalue = memory;
  } else {
    /* Memory already exists.  Just move the break */
    brk = sbrk(ask);
    if ( brk == (void*)-1) {
      logmsg(1,stderr,"FAILED.\n");
      return NULL;
    }
    /* it worked, hook it up. */
    memend->next = brk;
    memend->next->base = brk + HEADER_SIZE;
    memend->next->size = howmuch - HEADER_SIZE;
    memend->next->free = TRUE;
    memend->next->prev = memend;
    memend->next->next = NULL;
    if ( memend->free ) {
      merge_blocks(memend,memend->next);
    } else {
      memend=memend->next;
    }
    rvalue=memend;
  }

  logmsg(1,stderr,"ok.\n");

  return rvalue;
}

static int malloc_init(void) {
  /* set up the world as we'd like it to be */
  int ok = TRUE;
  char *debug, *end;

  /* check for debugging flag */
  if ( (debug=getenv("DEBUG_MALLOC")) ) {
    debug_malloc=strtol(debug, &end, 0);
    if ( debug_malloc == 0 )
      debug_malloc++;
    if ( debug_malloc < 0 || *end ) {
      /* if this was a bad number, complain and just set it to 1 */
      debug_malloc=TRUE;
      fprintf(stderr,
             "%s: invalid value for DEBUG_MALLOC.  Setting to %d.\n",
             debug, debug_malloc);
    }
  }

  /* move the break and allocate a hunk of memory to work with */
  memory=expand_break(0);

  if ( !memory )
    ok = FALSE;

  return ok;
}

static void split_block(mheader block, size_t size) {
  /* divide the given block into two blocks, one that can hold size
   * blocks (rounded to a multiple of MIN_BLOCK), and the rest.  Make
   * sure there's enough room to make it worth it.
   */
  mheader rest;

  /* round size up to nearest multiple of MIN_BLOCK */
  size = lmgt(MIN_BLOCK,size);

  if ( block->size >= size + (2*HEADER_SIZE) + MIN_BLOCK ) {
    /* There's room for a valid fragment; it's worth it; chop off rest. */
    /* create it */
    rest = block->base + size;

    /* cut it in */
    rest ->prev = block;
    rest ->next = block->next;
    if ( rest->next )
      rest->next->prev = rest;
    block->next = rest;
    /* set it up */
    rest ->free = TRUE;
    rest ->base = ((void*)rest) + HEADER_SIZE;
    rest ->size = block->size - size - HEADER_SIZE;
    block->size = size;
    if ( memend == block ) /* keep track of the end of the world */
      memend = rest;

    /* check to see if the new block can be merged w/its successor */
    if ( rest->next && rest->next->free )
      merge_blocks(rest, rest->next);
  }
}

static void merge_blocks(mheader one, mheader two){
  /* Merge the given blocks.  Insist that
   * (a) they be contiguous, and
   * (b) the second block must be free
   */
  if ( two->free && one->next == two ){ /* merge 'em */
    one->size = one->size + two->size + HEADER_SIZE;
    one->next = two->next;
    if (two->next)
      two->next->prev = one;
    /* keep  track of the end of the world */
    if ( memend == two )
      memend = one;
  } else {                      /* whoops */
    if ( !two->free )
      fprintf(stderr, "%s: attempt to merge allocated block", __FUNCTION__);
    else
      fprintf(stderr, "%s: attempt to merge non-contiguous blocks",
              __FUNCTION__);
  }
}

void *malloc(size_t size) {
  /* allocate the given amount of space. */
  void *rvalue;

  rvalue = internal_malloc(size);

  if ( debug_malloc ){
    mheader b=find_block_containing(memory,rvalue);
    logmsg(1,stderr, "MALLOC: malloc(%ld)\t=> (ptr=%p, size=%ld)\n",
            (long)size, rvalue, (long)(b?b->size:0));
    if ( debug_malloc > 2 )
      pm();
  }

  return rvalue;
}

static void *internal_malloc(size_t size) {
  /* allocate the given amount of space.  (Rounding takes place in
   * split_block().  This exists because malloc() is called by realloc()
   * and calloc().  We don't want to confuse the diagnostic output.
   * Returns the block containing the newly allocated memory.
   */
  mheader block, rest;
  void *rvalue;

  if ( (size < 0 ) ||                  /* if bad size  */
       (!memory && !malloc_init()) ) { /* or initialization fails */
    errno  = ENOMEM;                   /* bummer */
    rvalue = NULL;
  } if ( size == 0 ) {
    rvalue = NULL;       /* It's allowed... */
  } else {               /* find the right size block and return it */

    /* try and find a block.  If not, try to expand the heap until either
     * we find one or the expansion fails
     */
    if ( !(block = find_block(memory,size)) ) {
      do {        /* ask for this and a little bit more.
                   * repeat in case we don't get enough somehow
                   */
        rest = expand_break(size+CHUNK);
        block = find_block(rest,size);
      } while ( rest && !block );
    }

    if ( !block ) {             /* still no dice.  bummer. */
      rvalue = NULL;
      errno  = ENOMEM;
    } else {                    /* woohoo */
      block->free = FALSE;
      rvalue=block->base;
      split_block(block, size);
    }
  }

  return rvalue;
}

static void free_block(mheader block) {
  int ok;
  size_t amount, newsize;

  /* free a block, merging w/neighbors if possible */
  block->free = TRUE;
  /* check for possible merges with neighbors, next first, then
   * prev. This maintains the validity of the block pointer.
   */
  if ( block->next && block->next->free)
    merge_blocks(block, block->next);
  if ( block->prev && block->prev->free)
    merge_blocks(block->prev, block);

  /* If more than two CHUNKs now available at the high end of memory
   * some should be returned to the OS
   */
  if ( memend->free && memend->size > 2 * CHUNK ) {
    newsize = CHUNK + (memend->size % CHUNK);
    amount = memend->size - newsize;

    /* try to return amount to system */
    logmsg(1,stderr,"%s: trying to return %u bytes from break...",
           __FUNCTION__, (unsigned) amount);

    ok = (sbrk(-amount) != (void*) -1);
    if ( ok ) {
      /* it worked.  Yay.  Adjust our counters */
      memend->size = newsize;
      logmsg(1,stderr,"ok.\n");
    } else {
      /* it didn't work, but it's harmless */
      logmsg(1,stderr,"FAILED.\n");
    }

  }

}

void free(void *ptr) {
  /* Release the given memory, or do nothing if ptr is NULL */

  if ( ptr ) {                   /* if ptr is NULL, do nothing */
    logmsg(1, stderr, "MALLOC: free(0x%p)\n", ptr);
    if ( debug_malloc > 2 )
      pm();
    internal_free(ptr);
  } else if ( debug_malloc) {
    /* sigh.  Special-case this one because snprintf() free()s NULL */
    char *msg="MALLOC: free(NULL)\n";
    write(STDERR_FILENO, msg, strlen(msg));
  }
}

static void internal_free(void *ptr) {
  /* return the block containing this pointer to the wild
   * free() is also called by realloc() so, like malloc, we have
   * internal and external versions
   */
  mheader block;
  block = find_block_containing(memory, ptr);
  if ( block ) {
    free_block(block);
  } else {
    fprintf(stderr, "%s: freeing unallocated pointer!\n", __FUNCTION__);
  }

  return;
}


void *PLNcalloc(size_t nmemb, size_t elem_size){
  /* allocate memb elements of size size, cleared */
  void *rvalue;
  size_t size = nmemb*elem_size;

  if ( (rvalue = internal_malloc(size)) ) { /* get it  */
    memset(rvalue,0,size);                  /* wipe it if non-NULL */
  }

  if ( debug_malloc ) {
    if ( rvalue ) {
      mheader b=find_block_containing(memory, rvalue);
      logmsg(1, stderr, "MALLOC: calloc(%ld,%ld)\t=> (ptr=%p, size=%ld)\n",
              (long)nmemb, (long)elem_size, rvalue, (long)(b?b->size:0));
    } else {
      logmsg(1,stderr, "MALLOC: calloc(%ld,%ld)\t=> NULL\n", (long)nmemb,
              (long)elem_size);
    }
    if ( debug_malloc > 2 )
      pm();
  }

  return rvalue;
}

void *realloc(void *ptr, size_t size) {
  /* reallocate the given pointer to the given size, copying, if necessary.
   * (rounding takes place in split_block())
   */
  void *rvalue=NULL, *new;
  mheader block;

  if ( ptr == NULL ) {
    rvalue = internal_malloc(size);
  } else if ( size == 0 ) {
    internal_free(ptr);
    rvalue=NULL;                /* it's just neater this way */
  } else {
    block = find_block_containing(memory, ptr);
    if ( block && (size > 0) ) {
      /* This was allocated by us (and the new size makes sense).
       * Otherwise the default NULL will be returned.
       */

      /* try to extend this block, if possible (it can't hurt) */
      while ( block->next && block->next->free )
        merge_blocks(block, block->next);

      /* now, will it fit? */
      if ( size <= block->size ) {
        /* yes, woohoo! we don't have to move*/
        split_block(block,size);
        rvalue = block->base;
      } else {
        /* nope.  Get a new block and copy.  Block's size is smaller,
         * so copy block->size rather than size */
        if ( (new = internal_malloc(size)) ) {
          memcpy(new, block->base, block->size);
          rvalue=new;
          free_block(block);
        } else {                  /* no dice */
          rvalue = NULL;
        }
      }
    }
  }

  if ( debug_malloc ) {
    if ( rvalue ) {
      mheader b=find_block_containing(memory,rvalue);
      logmsg(1,stderr,
             "MALLOC: realloc(%p,%ld)\t=> (ptr=%p, size=%ld)\n",
             ptr, (long)size, rvalue, (long)(b?b->size:0));
    } else {
      logmsg(1,stderr, "MALLOC: realloc(%p,%ld)\t=> NULL\n", ptr,
             (long)size);
    }
    if ( debug_malloc > 2 )
      pm();
  }


  return rvalue;
}

#define powOf2(x) (((x != 0) && !(x & (x - 1))))

static size_t lmgt(size_t factor, size_t size) {
  /* LMGT--least multiple greater than.  Rounds size up
   * to the nearest multiple of factor
   */
    size_t extra;
    if ( powOf2(factor) )
      extra = size & (factor-1); /* mask if it's a power of two */
    else
      extra = size%factor;


    if (extra) {
      size = size-extra + factor;
    }
    return size;
}


/********************************************************************/
void print_memlist(FILE *where) {
  /* debugging code to print the memory list.  Since we only do thi
   * while debugging, it should be ok to use logmsg(1,...) here to
   * avoid printf() using malloc() and changing things.
   */
  mheader l;
  int entry=0;
  static int count=0;

  logmsg(1,where,"  Memory Map (time=%d  memory=%p  memend=%p):\n",
         count++, memory, memend);
  for(l=memory; l; l=l->next )
    logmsg(1, where,
           "    %0d-%p) {base=%p,size=%*ld,free=%d,prev=%p,next=%p}\n",
           entry++, l, l->base, PTRWID, (long int) l->size, l->free,
           l->prev, l->next);
}

void pm() {
  print_memlist(stderr);
}



#define LOGMSGSIZE 1024
static int logmsg(int level, FILE *stream, const char *format, ...) {
  /* print out logging messages.  This uses a fixed-size buffer
   * to be sure that snprintf() has no reason to call malloc()
   * (Although it still free()s NULL.  Why?)
   */
  char msg[LOGMSGSIZE];
  int rval=0;
  va_list ap;

  if ( debug_malloc >= level ) {
    va_start(ap, format);
    vsnprintf(msg, LOGMSGSIZE, format, ap);
    va_end(ap);
    rval = fputs(msg, stream);
  }

  return rval;
}

