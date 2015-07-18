
// Banking Simulation
// Fall 2014: CS 304 (Computer Organization)
// Professor: Gongbing Hong
// Made by: Divya Bathey

# include <stdio.h>
# include <string.h>
# include <pthread.h>
# include <stdlib.h>
# include <unistd.h>
# include <semaphore.h>

// STRUCT DEFINITIONS
typedef struct {
	int numTellers;		// Holds information from the user input
	int simTime;
	int serviceTime;
	int sampleTime;
	float probability;
} Information;
typedef struct {
	int *buf;		// buffer array
	int n;			// maximum number of slots
	int front;		// buf[ (front+1) %n ] is the first item
	int rear;		// buf[ rear%n ] is the last item
	sem_t mutex;	// protects access to buf
	sem_t slots;	// counts available slots
	sem_t items;	// counts available items
	sem_t served;
} line_t;

// GLOBAL VARIABLES
Information info;		// Needs to be accessible across functions
int timeCounter;		// Keeps track of time for all threads to use
int totalWait;			// Running counter of total wait times
pthread_mutex_t lock;	//    protected by this mutex lock

// FUNCTION PROTOTYPES (for both files)
int parseline(int argc, char *argv[]);
int genServeTime();
void addToTotal(int x);
void *arrivals(line_t *arg);
void *teller(line_t *arg);
void line_init(line_t *sp, int n);
void line_deinit(line_t *sp);
void line_insert(line_t *sp, int item);
int line_remove(line_t *sp);
