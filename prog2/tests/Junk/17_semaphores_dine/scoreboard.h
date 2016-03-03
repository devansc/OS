#ifndef SCOREBOARD_H
#define SCOREBOARD_H


/* tags for each status */

#define WAITING  0
#define EATING   1
#define THINKING 2

/* utility definitions */
#ifndef TRUE
#define TRUE (1==1)
#endif

#ifndef FALSE
#define FALSE (0==1)
#endif

/* extern declarations for external use */
extern void sb_init(int size);
extern void sb_destroy();
extern void sb_change_status(int who, int status);
extern void sb_takefork(int who, int which);
extern void sb_dropfork(int who, int which);
#endif
