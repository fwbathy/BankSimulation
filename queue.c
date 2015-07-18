
# include "queue.h"

// Banking Simulation
// Fall 2014: CS 304 (Computer Organization)
// Professor: Gongbing Hong
// Made by: Divya Bathey

// Queue structure:
// index-->   0    1   ...  ...  n-1
// slot-->   [ ]  [ ]  [ ]  [ ]  [ ]


// Create an empty, bounded, shared FIFO buffer with n slots
void line_init(line_t *sp, int n) {

	sp->buf = calloc(n, sizeof(int));		// Allocate space for buffer
	
	sp->n = n;						// Buffer holds up to n items
	sp->front = sp->rear = 0;		// Empty buffer IFF front == rear
	
	sem_init(&sp->mutex, 0, 1);		// A binary semaphore for locking
	sem_init(&sp->slots, 0, n);		// Buffer has n empty slots initially
	sem_init(&sp->items, 0, 0);		// Buffer has 0 items initially
	sem_init(&sp->served, 0, 0);	// Served 0 customers initially

}

// Clean up buffer sp
void line_deinit(line_t *sp) {
	free(sp->buf);
}

// Insert item onto the rear of shared buffer sp
void line_insert(line_t *sp, int item) {

	sem_wait(&sp->slots);	// Wait for available slot
	sem_wait(&sp->mutex);	// Lock the buffer

	sp->buf[ (++sp->rear) % (sp->n) ] = item;	// Insert item

	sem_post(&sp->mutex);	// Unlock the buffer
	sem_post(&sp->items);	// Announce available item

}

// Remove and return the first item from buffer sp
int line_remove(line_t *sp) {

	int item;

	sem_wait(&sp->items);	// Wait for available item
	sem_wait(&sp->mutex);	// Lock the buffer
	
	item = sp->buf[ (++sp->front) % (sp->n) ];	// Remove the item

	sem_post(&sp->mutex);	// Unlock the buffer
	sem_post(&sp->slots);	// Announce the available slot
	sem_post(&sp->served);	// Annouce a served customer

	return item;

}

