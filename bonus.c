
# include "queue.h"

// Banking Simulation
// Fall 2014: CS 304 (Computer Organization)
// Professor: Gongbing Hong
// Made by: Divya Bathey

// This program is designed to serve as a 'real-time' simulator that
// simulates bank customers waiting in line to be serviced by several
// tellers. Five command line inputs are parsed in for program use:
//     Simulation Time: total number of minutes to run the program
//     Arrival Probability: (0-1) chance of customer at 1-sec interval
//     Number of Tellers: specifies how many tellers to simulate
//     Customer Service Time: minutes a teller must service a customer;
// 		   given customer service time is an average -- actual service
//         time varies within a range of +/- 30 seconds
//     Sampling Time: how often  (in minutes) the simulator will 
//		   sample the queue length and print out data

// Multithreading is used in this simulation. The primary thread (main)
// creates all other threads and is responsible for printing information
// intermittently. A secondary thread is created to keep track of time
// and generate customer arrival events. Multiple threads simulate the
// bank tellers. A first-in first-out (FIFO) queue handles customers,
// which are represented by their arrival time. 

int main(int argc, char *argv[]) {	

	// Reads in arguments and sets info variables
	if(!parseline(argc, argv))
		fprintf(stderr, "Please input expected command line input.\n");
		exit(0);

	// Thread references
	pthread_t customerArrival;
	pthread_t tellers[info.numTellers];

	// Create the FIFO queue (line_t type); can hold max 100 customers
	line_t *waiting = malloc(sizeof *waiting);
	line_init(waiting, 10000);

	// Create arrival thread, passing in the queue 
	if(pthread_create(&customerArrival, NULL, arrivals, waiting)) {
		fprintf(stderr, "Error creating arrivals thread.\n");
		exit(0);
	}

	// Create teller threads, passing in the queue 
	int i;
	for(i = 0; i < info.numTellers; i++) {
		if(pthread_create(&tellers[i], NULL, teller, waiting)) {
			fprintf(stderr, "Error creating teller %d thread.\n", i);
			exit(0);
		}
	}

	// Print out information periodically as specified by the user
	int timePoint = 1;
	int stillInLine;
	while (timeCounter < ((info.simTime)*60) ) {
		if ( (timeCounter != 0) && ( (timeCounter % ((info.sampleTime)*60)) == 0 ) ) {
			sem_getvalue(&waiting->items, &stillInLine);
			printf("Time: %d      Queue Length: %d\n", timePoint, stillInLine);
			timePoint += 1;
			sleep(1);
		}
	}

	// Wait for arrival thread to finish
	if(pthread_join(customerArrival, NULL)) {
		fprintf(stderr, "Error joining thread\n");
		exit(0);
	}

	// Wait for teller threads to finish
	for(i = 0; i < info.numTellers; i++) {
		if(pthread_join(tellers[i], NULL)) {
			fprintf(stderr, "Error joining teller thread %d.\n",i);
			exit(0);
		}		
	}

	// Print out a summary of the simulation, including the
	// total number of customrs serviced and average wait time.
	int totalServed;
	sem_getvalue(&waiting->served, &totalServed);
	printf("\n");
	printf("A summary of the concluded simulation:\n");
	printf("--------------------------------------\n");
	printf("%d customers were served in total.\n", totalServed);
	printf("%d customers are still in line.\n", stillInLine);	
    // printf("Total wait time is %d\n", totalWait);
	float avg = ( (float)totalWait )/( (float)totalServed );
	printf("\n");		
	printf("The average customer waited for %1.3f seconds.\n",avg);
	printf("\n");	
	printf("\n");	
	printf("Process ended.\n");
	printf("\n");	
    
    // Destroy mutex lock after we're done using it.
    pthread_mutex_destroy(&lock);
    exit(0);
}


// Parseline reads in arguments and sets the global variables in the
// Information struct info to reflect user input. If something goes 
// wrong, it will return a 0 rather than a 1. 
// All arguments are expected to be greater than 0 and whole numbers 
// with the exception of the Arrival Probability, which is a float 0-1.
int parseline(int argc, char *argv[]) {
	
	int check = 1;

	// Wrong number of arguments.
	if(argc < 6 || argc > 6) {
		printf("Expected number of arguments: 5 \n");	
		printf("Given number of arguments: %d\n", argc-1);
		printf("Error: Incorrect number of arguments. Try again.\n");
		check = 0;
	}	

	else {

		// Set global Information struct 
		info.simTime = atoi(argv[1]);
		info.probability = atof(argv[2]); 
		info.numTellers = atoi(argv[3]);
		info.serviceTime = atoi(argv[4]);
		info.sampleTime = atoi(argv[5]);

		// Ensure that arguments were in fact numbers
		if(!info.simTime || !info.numTellers || !info.serviceTime || !info.sampleTime) {
			printf("Error: Arguments must be numbers greater than 0. Try again.\n");
			check = 0;
		}
		if(!info.probability || info.probability > 1 || info.probability < 0) {
			printf("Error: Probability must be a float value between 0 and 1. Try again.\n");
			check = 0;
		}

		// Print out variables to double check values.
		printf("\n");
		printf("You entered the following information:\n");
		printf("--------------------------------------\n");
		printf("Simulation time:       %d\n", info.simTime);
		printf("Probability:           %1.3f\n", info.probability);
		printf("Number of tellers:     %d\n", info.numTellers);
		printf("Service time:          %d\n", info.serviceTime);
		printf("Sampling time (min):   %d\n", info.sampleTime);
		printf("\n");

	}

	return check;
}


// Randomly generates a service time that is within 30 seconds of 
// the specified Customer Service Time.
int genServeTime() {
	// Randomly decide between adding/subtracting
	int add = rand() % 2;
	if(add) // add == 1 case --> addition
		return (info.serviceTime*60) + ( rand() % 30 );
	else 	// add == 0 case --> subtraction
		return (info.serviceTime*60) - ( rand() % 30 );
}


// The global variable totalWait is a running total of how long each
// serviced customer was waiting in line. It must be mutex-protected
// and this function allows a thread to add to the total safely.
void addToTotal(int x) {
	pthread_mutex_lock(&lock);
	totalWait = totalWait + x;
	pthread_mutex_unlock(&lock);
}


// This thread keeps track of time through timeCounter global variable.
// Additionally, it randomly generates arrival events and adds the
// customer arrival time to the queue as the customer ID.
void *arrivals(line_t *waiting) {

	// Begin program time at 0.
	timeCounter = 0;

	while(timeCounter < ( (info.simTime)*60 )) {

	    // printf("The time is now: %d\n", timeCounter);
		if( ((double) rand()) / ((double) RAND_MAX) < info.probability) {
			// Add 'customer' to queue upon arrival
			// printf("A customer with ID %d has arrived.\n", timeCounter);
			line_insert(waiting, timeCounter);
		}

		// One second pause before continuing.
		sleep(1);
		timeCounter += 1;

	}

	return NULL;
}


// Simulate bank teller. Pulls customers out of the queue (if there are
// any waiting) and 'services' them. Adds wait time to running total.
void *teller(line_t *waiting) {

	int custID;
	int service;
	int inLine;
	sem_getvalue(&waiting->items, &inLine);

	// For as long as the simulation is running, a teller will check
	// the queue for waiting customers (unless it's occupied).
	while(timeCounter < ( (info.simTime)*60 )) {
		if(inLine) {
			custID = line_remove(waiting);
			addToTotal(timeCounter-custID);
			service = genServeTime();

			// Pause for the duration of service.
			sleep(service);
		}
		sem_getvalue(&waiting->items, &inLine);
	}

	return NULL;
}
