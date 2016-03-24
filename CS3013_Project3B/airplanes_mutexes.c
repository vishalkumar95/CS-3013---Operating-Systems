/*Project 3B
Kevin Martin and Vishal Rathi
kpmartin and vkrathi
*/

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<semaphore.h>

//create pthread array and mutex locks
pthread_t tid[25];
pthread_mutex_t lock1, lock2, lock3;
#define AEROPLANES 25
#define RUNWAYS 3

//define airplanes and runways
struct Airplane {
	int id;
	int fuel;
	bool emergency;
	int arrived_status;
	int priority;
	int landing_flag;
	//rate = 1 fuel / sec
};

struct Runway {
	int id;
	bool occupied;
};

//create arrays for the runways and planes
struct Runway runways[RUNWAYS];
struct Airplane airplanes[AEROPLANES];

int emergencies;
int nonemergencies;

//fill the arrays
void init(){
	int i, j;	
	for(i = 0; i < RUNWAYS; i++) {
		runways[i].id = i;
		runways[i].occupied = false;
	}

	for(j = 0; j < AEROPLANES; j++) {
		airplanes[j].id = j;
		airplanes[j].fuel = rand() % 100 + 10;
		airplanes[j].landing_flag = 0;
		airplanes[j].arrived_status = 0;
		int random = rand() % 10;
		if(random == 0){
			airplanes[j].emergency = true;
		} else {
			airplanes[j].emergency = false;
		}
		airplanes[j].priority = 0;
	}
}

//land the plane with the given id on the given runway
void landPlane(int planeID, int runwayID) {

	airplanes[planeID].landing_flag = 1;
	if(airplanes[planeID].emergency == true) {	//emergency state
		printf("[Emergency plane] : Plane %d is beginning to land on runway %d\n   Fuel = %d\n", planeID, runwayID, airplanes[planeID].fuel);
		int landTime = rand() % 5 + 1;
		//if the plane runs out of fuel while landing
		if(landTime > airplanes[planeID].fuel){
			printf("[Emergency plane crash] : Plane %d has run out of fuel and crashed\n", planeID);
			exit(0);
		}
		sleep(landTime);
		if ((airplanes[planeID].fuel - landTime) < 40){
			printf("[Emergency plane] : Plane %d has entered the danger zone\n", planeID);
		}
		printf("[Emergency plane] : Plane %d has landed on runway %d\n   Fuel = %d\n", planeID, runwayID, airplanes[planeID].fuel - landTime);
		int clearTime = rand() % 20 + 1;	
		sleep(clearTime);
		printf("[Emergency plane] : Plane %d has cleared the runway %d\n   Fuel = %d\n", planeID, runwayID, airplanes[planeID].fuel - landTime);
		//decrement the number of emergency planes in the airspace
		emergencies--;

	} else {	//regular state
		printf("[Plane Landing] : Plane %d is beginning to land on runway %d\n   Fuel = %d\n", planeID, runwayID, airplanes[planeID].fuel);
		
		int landTime = rand() % 5 + 1;
		//if the plane runs out of fuel while landing
		if(landTime > airplanes[planeID].fuel){
			printf("[Plane crash] : Plane %d has run out of fuel and crashed\n", planeID);
			exit(0);
		}
		sleep(landTime);
		if ((airplanes[planeID].fuel - landTime) < 40){
			printf("[Emergency plane] : Plane %d has entered the danger zone\n", planeID);
		}
		printf("[Plane Landing] : Plane %d has landed on runway %d\n   Fuel = %d\n", planeID, runwayID, airplanes[planeID].fuel - landTime);
		
		int clearTime = rand() % 20 + 1;	
		sleep(clearTime);
		printf("[Plane Landing] : Plane %d has cleared the runway %d\n   Fuel = %d\n", planeID, runwayID, airplanes[planeID].fuel - landTime);
	}

	//mutex lock for the runways
	if(runwayID == 0){
		pthread_mutex_lock(&lock1);
		runways[runwayID].occupied = false;
		pthread_mutex_unlock(&lock1);
	} else if(runwayID == 1) {
		pthread_mutex_lock(&lock2);
		runways[runwayID].occupied = false;
		pthread_mutex_unlock(&lock2);
	} else if(runwayID == 2){
		pthread_mutex_lock(&lock3);
		runways[runwayID].occupied = false;
		pthread_mutex_unlock(&lock3);
	}
	
	return;
}

//spawn the plane with the given id
void* spawnPlane(void *plane){
	long planetid;
	int flag = 0;
	int i;	
	int planeID = ((struct Airplane*)plane)->id;
	if(airplanes[planeID].emergency == true) {	//emergency state
		printf("[Plane Arriving] : Plane %d has arrived into the airspace.\n   Fuel = %d\n", planeID, airplanes[planeID].fuel);
		printf("[Plane Arriving] : Plane %d has declared an emergency state \n", planeID);
		emergencies++;	//increment number of emergency planes
		airplanes[planeID].priority = 1;
	} else { //regular state
		printf("[Plane Arriving] : Plane %d has arrived into the airspace.\n   Fuel = %d\n", planeID, airplanes[planeID].fuel);
	}
	airplanes[planeID].arrived_status = 1;

	int c = 0;
	int d = 0;
	struct Airplane temporary;
	temporary.emergency = false;
	temporary.fuel = 10000;
	temporary.landing_flag = 0;
	temporary.arrived_status = 1;
	for( c = 0; c < AEROPLANES; c++ ) {
			if( ((temporary.fuel > airplanes[c].fuel) && (airplanes[c].landing_flag == 0)) && (airplanes[c].arrived_status == 1) ) {
				temporary = airplanes[c];
				d = c;
			}

	}
	airplanes[d].priority = 1;	

	
	while(airplanes[planeID].fuel > 1){	//plane needs at least 1 fuel units to land
		//loop through planes, if emergency plane in the airspace, dont begin landing		
		for(i = 0; i < airplanes[planeID].fuel; i++) {
			if((emergencies > 0 && airplanes[planeID].emergency == 0) || (airplanes[planeID].priority != 1)) {
				airplanes[planeID].fuel--;
				sleep(1);
			} else {	//begin landing
				if(runways[0].occupied == false){
						pthread_mutex_lock(&lock1);
						runways[0].occupied = true;
						pthread_mutex_unlock(&lock1);
						landPlane(planeID, 0);
						flag = 1;
						return;
					//}
				} else if(runways[1].occupied == false){
						pthread_mutex_lock(&lock2);
						runways[1].occupied = true;
						pthread_mutex_unlock(&lock2);
						landPlane(planeID, 1);
						flag = 1;
						return;
					//}
				} else if(runways[2].occupied == false){
						pthread_mutex_lock(&lock3);
						runways[2].occupied = true;
						pthread_mutex_unlock(&lock3);
						landPlane(planeID, 2);
						flag = 1;
						return;
					//}
				} else {
					sleep(1);
					airplanes[planeID].fuel--;
				}
			}
		}		
	}
	//should only get here if a plane runs out of fuel
	if (flag == 0) {
		printf("[Plane crash] : Plane %d has run out of fuel and crashed\n", planeID);
		exit(0);
	}
}

int main(void){
	int i = 0;
	int j = 0;
	int err;

	srand(time(NULL));

	printf("initializing\n");
	init();
	
	//Initialize the mutexes
	if (pthread_mutex_init(&lock1, NULL) != 0){
		printf("\n semaphore init failed\n");
		return 1;
	}
	if (pthread_mutex_init(&lock2, NULL) != 0){
		printf("\n semaphore init failed\n");
		return 1;
	}
	if (pthread_mutex_init(&lock3, NULL) != 0){
		printf("\n semaphore init failed\n");
		return 1;
	}
	
	while(i < AEROPLANES){
		err = pthread_create(&(tid[i]), NULL, spawnPlane, (void *)&airplanes[i]);
		if (err != 0){
			printf("\ncan't create thread :[%s]", strerror(err));
		}
		i++;
		int sleepTime = rand() % 5;
		sleep(sleepTime);
	}
	while (j < AEROPLANES){
		pthread_join(tid[j], NULL);
		j++;
	}
	pthread_mutex_destroy(&lock1);
	pthread_mutex_destroy(&lock2);
	pthread_mutex_destroy(&lock3);

	return 0;
}

