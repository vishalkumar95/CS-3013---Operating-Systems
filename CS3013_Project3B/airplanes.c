#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>

//create pthread array and mutex locks
pthread_t tid[25];
pthread_mutex_t lock1, lock2, lock3;

//define airplanes and runways
struct Airplane {
	int id;
	int fuel;
	bool emergency;
};

struct Runway {
	int id;
	bool occupied;
};

//create arrays for the runways and planes
struct Runway runways[3];
struct Airplane airplanes[25];

//fill the arrays
void init(){
	int i, j;	
	for(i = 0; i < sizeof runways; i++) {
		runways[i].id = i;
		runways[i].occupied = 0;
	}

	for(j = 0; j < sizeof airplanes; j++) {
		airplanes[j].id = j;
		airplanes[j].fuel = rand() % 40 + 10;
		int random = rand() % 10;
		if(random = 0){
			airplanes[j].emergency = true;
		} else {
			airplanes[j].emergency = false;
		}
	}
}

//land the plane with the given id on the given runway
void landPlane(int planeID, int runwayID)
{
	printf("Plane %d is beginning to land on runway %d\n   Fuel = %d\n", planeID, runwayID, airplanes[planeID].fuel);
	sleep(3);
	printf("Plane %d has landed on runway %d\n   Fuel = %d\n", planeID, runwayID, airplanes[planeID].fuel - 3);
	sleep(2);
	printf("Plane %d has cleard the runway %d\n   Fuel = %d\n", planeID, runwayID, airplanes[planeID].fuel);
	
	if(runwayID == 1){
		pthread_mutex_unlock(&lock1);
	} else if(runwayID == 2) {
		pthread_mutex_unlock(&lock2);
	} else if(runwayID == 3){
		pthread_mutex_unlock(&lock3);
	}
    pthread_join(tid[planeID], NULL);
}

//spawn the plane with the given id
void* spawnPlane(void *plane){
	int planeID;
	planeID = ((struct Airplane*)plane)->id;
	printf("Plane %d has arrived.\n   Fuel = %d", planeID, airplanes[planeID].fuel);
	int i;	
	while(airplanes[planeID].fuel > 2){	//plane needs 3 fuel units to land
		if(pthread_mutex_lock(&lock1) == 0){ // check lock1
			landPlane(planeID, 1);
			return;
		} else if(pthread_mutex_lock(&lock2) == 0){ // check lock2
			landPlane(planeID, 2);
			return;
		} else if(pthread_mutex_lock(&lock3) == 0){ // check lock3
			landPlane(planeID, 3);
			return;
		} else {
			sleep(1);
			airplanes[planeID].fuel--;
		}
		
	}
	printf("Plane %d has run out of fuel and crashed\n", planeID);
	return;
}

int main(void)
{
    int i = 0;
    int err;

	printf("initializing\n");
	init();
	
	//create a lock for each runway
	if (pthread_mutex_init(&lock1, NULL) != 0){
        printf("\n mutex init failed\n");
        return 1;
    }
	if (pthread_mutex_init(&lock2, NULL) != 0){
        printf("\n mutex init failed\n");
        return 1;
    }
	if (pthread_mutex_init(&lock3, NULL) != 0){
        printf("\n mutex init failed\n");
        return 1;
    }
	
	while(i < 25){
        err = pthread_create(&(tid[i]), NULL, spawnPlane, (void *)&airplanes[i]);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        i++;
		int sleepTime = rand() % 5;
		sleep(sleepTime);
    }

    pthread_mutex_destroy(&lock1);
	pthread_mutex_destroy(&lock2);
	pthread_mutex_destroy(&lock3);

    return 0;
}
