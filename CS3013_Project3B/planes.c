/* Includes */
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <semaphore.h>	// Semaphore library to access semaphore functions	

// This struct stores information for each plane. Each plane is given a number and the thread id is stored.
typedef struct {
	int plane_number;		// The number of the plane
	pthread_t tid;			// The thread id of the plane
	int landing_time;		// Amount of time required for the plane to land
	int runway_time;		// Time for the plane to clear the runway
	int fuel;			// Amount of fuel left when the plane enters the airspace
	int rate;			// Fuel depletion rate for the plane
	int emergency_situation;	// Flag variable to see if the plane is in emergency condition
	int priority;			// Integer to set the priority for each plane
	int fuel_remaining;
} planes;

sem_t runway;				// Semaphore to keep track of the runways
sem_t airplane_queue;			// Semaphore for the airplane queue
sem_t airplane_mutex;			// Semaphore to keep track of the strctures of the plane

planes *store_planes;
void swap(int *xp, int *yp);
void bubbleSort(int arr[], int n, int arr1[]);

// Function initializations
void *plane_land(void *args);

int main(int argc, char* argv[]) {
	srand(time(NULL));
	// Intitialize the struct containing the information for each plane
	int i;
	store_planes = (planes *) malloc(sizeof(planes) * 25);
	for (i = 0; i < 25; i++){
		store_planes[i].plane_number = i + 1;
		store_planes[i].landing_time = (rand() % (5 - 1)) + 1;
		store_planes[i].runway_time = (rand() % (150 - 50)) + 50;
		store_planes[i].fuel = (rand() % (50 - 10)) + 10;
		store_planes[i].rate = 2;
		store_planes[i].emergency_situation = 0;
		store_planes[i].priority = 0;
	}

	// Store thread ids for the 25 aeroplanes in an array
	int j;
	//pthread_t t[25];
	int thread_status;
	for (j = 0; j < 25; j++) {
		thread_status = pthread_create(&store_planes[j].tid, NULL, plane_land, (void *)&store_planes[j]);
		if (thread_status != 0) {
			printf("pthread_create");
			exit(1);
		}
	}

	// Loop to join the threads; pthread_join
	int k;
	for (k = 0; k < 25; k++) {
		pthread_join(store_planes[k].tid,NULL);
	}
	
	return 0;
}

void *plane_land(void *args) {
	
	long plane_tid;
	plane_tid = (long)(((planes*)args)->plane_number);
	
	// Random generator to make the thread sleep	
	int r = (rand() % (20 - 1)) + 1;
	int status = sleep(r);		// Make the thread sleep for r seconds

	// Check if the plane is in the danger zone
	int fuel_left;
	int depletion_rate;
	int time;
	int fuel_required;
	int fuel_remaining;
	fuel_left = (int)(((planes*)args)->fuel);
	depletion_rate = (int)(((planes*)args)->rate);	
	time = (int)(((planes*)args)->landing_time);
	fuel_required = time*depletion_rate;
	fuel_remaining = fuel_left - fuel_required;

	if (status == 0) {
		(((planes*)args)->priority) = -1;	// Setting the prioirty to -1 showing that the plane has arrived in the airspace
		(((planes*)args)->fuel_remaining) = fuel_remaining;
		printf("[planes - arriving]: Aeroplane %ld is arriving in the airspace now \n", plane_tid);
	}

	// Checking if the fuel remaining is less than the threshold for the danger zone
	if (fuel_remaining < 12) {
		printf("[planes - dangerzone]: Aeroplane %ld is entering the danger zone now \n", plane_tid);
	}

	// Declaring the emergency state for the plane
	int s = (rand() % (20 - 1)) + 1;
	int declare_emergency;
	if (s == 5) {
		declare_emergency = 1;
		(((planes*)args)->emergency_situation) = declare_emergency;
		(((planes*)args)->priority) = -2;	// Setting the priority showing that the plane has declared an emergency state
		printf("[planes - emergency]: Aeroplane %ld has declared an emergency state \n", plane_tid);
	}

	// Loop to store the fuels and plane ids of each plane in an array
	int i;
	int fuel_array[25];
	int plane_id[25];
	for (i = 0; i < 25; i++){
		if (((store_planes[i].priority) == -1) && ((store_planes[i].plane_number) == plane_tid)){
			fuel_array[i] = store_planes[i].fuel_remaining;
			plane_id[i] = plane_tid;
		}

		else if ((((store_planes[i].priority) == -2) && ((store_planes[i].emergency_situation) == 1)) && ((store_planes[i].plane_number) == plane_tid)){
			fuel_array[i] = store_planes[i].fuel_remaining;
			plane_id[i] = plane_tid;
		}

		else {
			fuel_array[i] = 1000;
			plane_id[i] = 1000;
		}
	}

	// Run sort algorithm on the fuel array
	bubbleSort(fuel_array, 25, plane_id);
	int n;
	for (n = 0; n < 25; n++){
		int z = (rand() % (10 - 1)) + 1;
		int status1 = sleep(z);		// Make the thread sleep for z seconds
		if ((plane_id[n] != 1000) && status1 == 0){
			printf("[planes - landing]: Aeroplane %d is ready to land\n", plane_id[n]);
		}
	}
		  			
	// Set the priority for each plane
	int k;
	for (k = 0; k < 25; k++) {
		int temp = plane_id[k];
		if ((store_planes[temp].emergency_situation) == 0){
			(store_planes[temp].priority) = k + 1;
			//printf("[planes - landing]: Aeroplane %d is ready to land\n", store_planes[temp].plane_number);
		}
		else if ((store_planes[temp].emergency_situation) == 1){
			int m;
			for (m = 0; m < 25; m++){
				if (m == temp){
					store_planes[m].priority = 1;
					//printf("[planes - landing]: Aeroplane %d is ready to land\n", store_planes[temp].plane_number);
				}
				else {
					store_planes[m].priority = ((store_planes[temp].priority) - 1);
					//printf("[planes - landing]: Aeroplane %d is ready to land\n", store_planes[temp].plane_number);
				}
			}
		}
	}
		
	pthread_exit(NULL);
}

// Swap the two values
void swap(int *xp, int *yp){
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}
 
// A function to implement bubble sort
void bubbleSort(int arr[], int n, int arr1[]){
	int i, j;
   	for (i = 0; i < n-1; i++){      
       		for (j = 0; j < n-i-1; j++){ //Last i elements are already in place  
           		if (arr[j] > arr[j+1]){
              			swap(&arr[j], &arr[j+1]);
	      			swap(&arr1[j], &arr1[j+1]);
	   		}
		}
	}
}

