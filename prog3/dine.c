#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5
#endif

#define NO_FORK -1
#define COL_WIDTH 8

/* States the philosophers can be in */
typedef enum STATE {
    EATING, THINKING, CHANGING
} STATE;

/* Struct describing a philosopher */
typedef struct PhilState {
    STATE state;
    int leftFork;
    int rightFork;
    int numRounds;
} PhilState;

/* Globals */
pthread_mutex_t *forks;
pthread_mutex_t printingLock;
PhilState *philosopherStates;
int numRounds;

/* Initializes a philosopher to CHANGING state and no forks */
PhilState *initState() {
    PhilState *state = malloc(sizeof(PhilState));
    state->state = CHANGING;
    state->leftFork = NO_FORK;
    state->rightFork = NO_FORK;
    state->numRounds = numRounds;
    return state;
}

/* Initializes the random call with system's microsecond counter */
void initRandom() {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    srandom((unsigned int) tv.tv_usec);
    printf("Initializing random with %u\n", (unsigned int) tv.tv_usec);
}

/* Sleeps a thread for a random amount of time */
void dawdle() {
    struct timespec tv;
    int msec;
    
    msec = (int)(((double)random() / INT_MAX) * 1000);
    tv.tv_sec = 0;
    tv.tv_nsec = 1000000 * msec;
    if (-1 == nanosleep(&tv, NULL)) {
        perror("nanosleep");
    }
}

/* Locks a semaphore and reports error if there is one. */
void lock(pthread_mutex_t *semaphore) {
    if (pthread_mutex_lock(semaphore)) {
        perror("pthread_mutex_lock");
        exit(-1);
    }
}

/* Unlocks a semaphore and reports error if there is one. */
void unlock(pthread_mutex_t *semaphore) {
    if (pthread_mutex_unlock(semaphore)) {
        perror("pthread_mutex_unlock");
        exit(-1);
    }
}

/* Creates and returns a string of the forks held in the philosophers hand */
char *getForkString(PhilState state) {
    char *string = malloc(NUM_PHILOSOPHERS);
    memset(string, (int)'-', NUM_PHILOSOPHERS);
    if (state.rightFork != NO_FORK)
        string[state.rightFork] = '0' + state.rightFork;
    if (state.leftFork != NO_FORK)
        string[state.leftFork] = '0' + state.leftFork;
    return string;
}

/* Prints the forks held and state of one philosopher */
void printState(PhilState state) {
    char *forkString = getForkString(state);
    switch (state.state) {
    case CHANGING:
            printf("| %s       ", forkString);
        break;
    case EATING:
            printf("| %s Eat   ", forkString);
        break;
    case THINKING:
            printf("| %s Think ", forkString);
        break;
    }
    free(forkString);
}

/* Prints a line of all the states of the philosophers */
void printStates() {
    int i;
    lock(&printingLock);
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        printState(philosopherStates[i]);
    }
    printf("|\n");
    unlock(&printingLock);
}

/* Locks and sets the philosophers right fork to numFork */
void pickupRightFork(int numPhil, int numFork) {
    lock(&forks[numFork]);
    philosopherStates[numPhil].rightFork = numFork;
    printStates();
}

/* Locks and sets the philosophers left fork to numFork */
void pickupLeftFork(int numPhil, int numFork) {
    lock(&forks[numFork]);
    philosopherStates[numPhil].leftFork = numFork;
    printStates();
}

/* Unlocks and drops the philosophers right fork - sets the value to NO_FORK */
void dropRightFork(int numPhil, int numFork) {
    philosopherStates[numPhil].rightFork = NO_FORK;
    unlock(&forks[numFork]);
    printStates();
}

/* Unlocks and drops the philosophers left fork - sets the value to NO_FORK */
void dropLeftFork(int numPhil, int numFork) {
    philosopherStates[numPhil].leftFork = NO_FORK;
    unlock(&forks[numFork]);
    printStates();
}

/* Changes a philosophers state to newState */
void changeState(int numPhil, STATE newState) {
    philosopherStates[numPhil].state = newState;
    printStates();
}

/* Handler for the even philosopher - picks up right then left fork */
void *evenPhilosopher(void *num) {
    int numPhil = (*(int *) num);
    int right = (numPhil + 1) == NUM_PHILOSOPHERS ? 0 : numPhil + 1;
    int left = numPhil;
    int rounds = philosopherStates[numPhil].numRounds;

    printStates();
    while (rounds-- > 0) {
        pickupRightFork(numPhil, right);
        pickupLeftFork(numPhil, left);
        changeState(numPhil, EATING);
        dawdle();
        changeState(numPhil, CHANGING);
        dropRightFork(numPhil, right);
        dropLeftFork(numPhil, left);
        changeState(numPhil, THINKING);
        dawdle();
        changeState(numPhil, CHANGING);
    }

    return NULL;
}

/* Handler for the even philosopher - picks up left then right fork */
void *oddPhilosopher(void *num) {
    int numPhil = (*(int *) num);
    int right = (numPhil + 1) == NUM_PHILOSOPHERS ? 0 : numPhil + 1;
    int left = numPhil;
    int rounds = philosopherStates[numPhil].numRounds;

    printStates();
    while (rounds-- > 0) {
        pickupLeftFork(numPhil, left);
        pickupRightFork(numPhil, right);
        changeState(numPhil, EATING);
        dawdle();
        changeState(numPhil, CHANGING);
        dropLeftFork(numPhil, left);
        dropRightFork(numPhil, right);
        changeState(numPhil, THINKING);
        dawdle();
        changeState(numPhil, CHANGING);
    }

    return NULL;
}

/* Prints the footer of the output */
void printFooter() {
    int i;
    char *string = malloc(NUM_PHILOSOPHERS + COL_WIDTH + 1);
    memset(string, (int)'=', NUM_PHILOSOPHERS + COL_WIDTH);
    string[NUM_PHILOSOPHERS + COL_WIDTH] = 0;
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("|%s", string);
    }
    printf("|\n");
    free(string);
}

/* Prints the header of the output */
void printHeader() {
    int i;
    int numSpaces = NUM_PHILOSOPHERS + COL_WIDTH - 1;
    int leftSpaces = numSpaces / 2;
    int rightSpaces = numSpaces / 2 + (numSpaces % 2);

    printFooter();

    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("| %*c%*s", leftSpaces, 'A' + i, rightSpaces, "");
    }
    printf("|\n");

    printFooter();
}

int main(int argc, char **argv) {
    int i;
    int id[NUM_PHILOSOPHERS];
    pthread_t philid[NUM_PHILOSOPHERS];
    forks = calloc(sizeof(pthread_mutex_t), NUM_PHILOSOPHERS);
    philosopherStates = calloc(sizeof(PhilState), NUM_PHILOSOPHERS);

    /* Sets numRounds to 2nd arg or 1 if not available */
    if (argc > 2) {
        printf("usage: ./dine [times around]\n");
        return -1;
    } else if (argc == 2) {
        numRounds = strtoimax(argv[1], NULL, 0);
        if (numRounds == 0) {
            printf("usage: ./dine [times around]\n");
            return -1;
        }
    } else {
        numRounds = 1;
    }

    /* Initialize the random calls */
    initRandom();

    printHeader();

    /* Initialize all globals */
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
        philosopherStates[i] = *initState(i);
    for (i = 0; i < NUM_PHILOSOPHERS; i++)
        if (pthread_mutex_init(&forks[i], NULL)) perror("pthread_mutex_init");
    if (pthread_mutex_init(&printingLock, NULL)) perror("pthread_mutex_init");

    for (i = 0; i < NUM_PHILOSOPHERS; i++)
        id[i] = i;

    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        int res;
        
        res = pthread_create(
            &philid[i],
            NULL,
            i % 2 ? oddPhilosopher: evenPhilosopher, 
            (void *) (&id[i])
        );

        if (res == -1) { 
            perror("pthread_create");
            return -1;
        }
    }

    /* Waits for all philosopher threads to finish */
    for (i = 0; i < NUM_PHILOSOPHERS; i++) {
        if (pthread_join(philid[i], NULL)) perror("pthread_join");
    }

    printFooter();

    return 0;
}
