/* a scoreboarding system for hungry philosophers
 *
 * A Philosopher's status will either be eating, thinking, or waiting,
 * represented by "E", "T", or " ".  A philosopher can be holding one
 * or two forks.
 *
 * Each time a status change occurs, a new status line is printed.

 * we don't need to lock since lwp is non-preemptive
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "scoreboard.h"
#include "util.h"

static void print_stat(int who);
static void sb_print_status();

static char *name[] = {"","Eat","Think"};

struct sb_stat {
  int status;                   /* status of philosopher */
  int *fork;                    /* fork-vector */
};

static struct sb_stat *sb;
static int sb_size;

extern void sb_init(int size) {
  /* set up the scoreboard system */
  int i,j;

  sb_size = size;

  sb=(struct sb_stat*)safe_malloc(sb_size * sizeof(struct sb_stat));
  for(i=0;i<sb_size;i++) {
    sb[i].status = WAITING;
    sb[i].fork   = (int*)safe_malloc(sb_size * sizeof(int));
    for(j=0;j<sb_size;j++) {
      sb[i].fork[j]=FALSE;
    }
  }


  /* after each is initialized, print the titles*/
  for(i=0;i<sb_size;i++)
    printf("|=============");
  printf("|\n");
  for(i=0;i<sb_size;i++)
    printf("|      %c      ",i+'A');
  printf("|\n");
  for(i=0;i<sb_size;i++)
    printf("|=============");
  printf("|\n");

  sb_print_status();

}

static void print_stat(int who) {
  int i;
  printf("| ");
  for(i=0;i<sb_size;i++) {
    if ( sb[who].fork[i] )
      printf("%d",i);
    else
      printf("-");
  }
  printf(" %-5s ",name[sb[who].status]);
}

extern void sb_destroy() {
  /* free everything and close out the scoreboard */
  int i;

  for(i=0;i<sb_size;i++){
    free(sb[i].fork);
    printf("|=============");
  }
  free(sb);
  printf("|\n");
}

static void sb_print_status(){
  /* should only be called by someone holding the statuslock */
  int i;

  for(i=0;i<sb_size;i++)
    print_stat(i);
  printf("|\n");
}

extern void sb_change_status(int who, int status) {
  sb[who].status = status;
  sb_print_status();
}

extern void sb_takefork(int who, int which) {
  sb[who].fork[which] = TRUE;
  sb_print_status();
}

extern void sb_dropfork(int who, int which) {
  sb[who].fork[which] = FALSE;
  sb_print_status();
}
