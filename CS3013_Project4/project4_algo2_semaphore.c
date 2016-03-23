// Vishal Rathi

// This file implements the LRU algorithm.

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<stdint.h>
#include<semaphore.h>

#define Page_Table_Capacity 1000
#define SSD_Capacity 100
#define HD_Capacity 1000
#define RAM_Capacity 25

typedef signed short vAddr;

vAddr create_page();
uint32_t get_value(vAddr address, int *valid);
void store_value(vAddr address, uint32_t *value);
void free_page(vAddr address);
void setRAM();
int findfree(int capacity);
void memoryMaxer();
int pagefaulthandler(vAddr address);
int RAMLRUEvict();
int LRU(int capacity);
void setSSD();
void setHD();
void testmethod();
void testMethod2();
void initThreads();
void *thread_process(void *arg);

int page_count = 0;
int mod_count = 1;

sem_t page_create;	// Semaphore for the page_create() function
sem_t value_get;	// Semaphore for the get_value() function
sem_t value_store;	// Sempahore for the store_value() function
sem_t page_free;	// Sempahore for the free_page() function
sem_t lock_page;	// Sempahore for the multi-threads

pthread_t tid[10];	// Store the thread ids
int threads_count[5];	// Store the thread number

typedef struct {
	int modified;
	int referenced;
	int mem_location;
	vAddr virtual_address;
	int physical_address;
	int valid;
	int flag;
}page_table_entry;

uint32_t SSD[SSD_Capacity];
uint32_t HD[HD_Capacity];
uint32_t RAM[RAM_Capacity];

page_table_entry page_table[Page_Table_Capacity];

int main(){

	//Initialize the semaphores
	if (sem_init(&page_create, 0, 1) != 0){
		printf("\n semaphore init failed\n");
		return 1;
	}
	if (sem_init(&value_get, 0, 1) != 0){
		printf("\n semaphore init failed\n");
		return 1;
	}
	if (sem_init(&value_store, 0, 1) != 0){
		printf("\n semaphore init failed\n");
		return 1;
	}
	
	if (sem_init(&page_free, 0, 1) != 0){
		printf("\n semaphore init failed\n");
		return 1;
	}
	
	if (sem_init(&lock_page, 0, 1) != 0){
		printf("\n semaphore init failed\n");
		return 1;
	}

	// Initialize my page_entry structure

	int i;

	for (i = 0; i < 1000; i++){
		page_table[i].physical_address = -1;
		page_table[i].modified = 0;
		page_table[i].referenced = 0;
		page_table[i].mem_location = 0;
		page_table[i].virtual_address = i;
		page_table[i].valid = 0;
		page_table[i].flag = 0;
	}

	setRAM();
	setSSD();
	setHD();

	struct timeval startTime, endTime;
	gettimeofday(&startTime, NULL);
	memoryMaxer();
	gettimeofday(&endTime, NULL);
	// Print time
	unsigned long long tdiffe = (((unsigned long long)endTime.tv_sec * 1000) + ((unsigned long long)endTime.tv_usec / 1000));
	unsigned long long tdiffs = (((unsigned long long)startTime.tv_sec * 1000) + ((unsigned long long)startTime.tv_usec / 1000));
	unsigned long long tdiff = tdiffe - tdiffs;
	printf("\n\n\n memoryMaxer() took %lld\n", tdiff);

	// More
	gettimeofday(&startTime, NULL);
	testmethod();
	gettimeofday(&endTime, NULL);
	// Print time
	unsigned long long tdiff2e = (((unsigned long long)endTime.tv_sec * 1000) + ((unsigned long long)endTime.tv_usec / 1000));
	unsigned long long tdiff2s = (((unsigned long long)startTime.tv_sec * 1000) + ((unsigned long long)startTime.tv_usec / 1000));
	unsigned long long tdiff2 = tdiff2e - tdiff2s;
	printf("\n\n\n testmethod() took %lld\n", tdiff2);

	// More
	gettimeofday(&startTime, NULL);
	testMethod2();
	gettimeofday(&endTime, NULL);
	// Print time
	unsigned long long tdiff3e = (((unsigned long long)endTime.tv_sec * 1000) + ((unsigned long long)endTime.tv_usec / 1000));
	unsigned long long tdiff3s = (((unsigned long long)startTime.tv_sec * 1000) + ((unsigned long long)startTime.tv_usec / 1000));
	unsigned long long tdiff3 = tdiff3e - tdiff3s;
	printf("\n\n\n testMethod2() took %lld\n", tdiff3);
	//initThreads();

	// Destroy the semaphores
	sem_destroy(&page_create);
	sem_destroy(&value_get);
	sem_destroy(&value_store);
	sem_destroy(&page_free);
	sem_destroy(&lock_page);

	return 0;
}


void setRAM(){
	int i;
	for (i = 0; i < RAM_Capacity; i++){
		RAM[i] = -1;
	}
}

void setSSD(){
	int i;
	for (i = 0; i < SSD_Capacity; i++){
		SSD[i] = -1;
	}
}

void setHD(){
	int i;
	for (i = 0; i < HD_Capacity; i++){
		HD[i] = -1;
	}
}

/*This function reserves a new memory location, which is 32 bits in size. This
memory block must be created in the emulated RAM, pushing other pages out of the emulated RAM
into lower layers of the hierarchy, if needed. Returns -1 if no memory is available.
*/
vAddr create_page(){
	
	sem_wait(&page_create);		// Do a sem_wait for mutual exclusion
	// Check if the page count exceeded 1000
	if (page_count == 1000){
		printf("There is not enough room left to create a new page entry \n");
	}

	// Find an empty slot in the page table
	
	int z = 0;
	vAddr address;
	for (z = 0; z < Page_Table_Capacity; z++){
		if (page_table[z].physical_address == -1){
			address = z;
			page_count++;
			break;
		}
	}

	// This loop will find an empty slot in the RAM
	int j;
	int RAM_slot = -1;
	for (j = 0; j < RAM_Capacity; j++){
		if (RAM[j] == -1){
			RAM_slot = j;
			break;
		}
	}

	// If we didn't find a spot,run the page eviction algorithm. This will bring us an index to an empty spot in the RAM
	if (RAM_slot == -1) {
		RAM_slot = RAMLRUEvict();
		if (RAM_slot == -10){
			printf("We can not accomodate the page \n");
			sem_post(&page_create);		// Do a sem_post for mutual exclusion
			return address;
		}
	}
	
	usleep(0.01*1000000);
	RAM[RAM_slot] = 4;
	page_table[address].mem_location = 1;
	page_table[address].valid = 1;
	page_table[address].physical_address = RAM_slot;
	page_table[address].modified = mod_count;
	page_table[address].referenced = mod_count;
	page_table[address].flag = 1;

	// Update
	int s = 0;
	for (s = 0; s < Page_Table_Capacity; s++){
		if ((s != address) && (page_table[s].valid == 1)) {
			page_table[s].flag++;
			page_table[s].referenced++;
		}
	}
	
	sem_post(&page_create);		// Do a sem_post for mutual exclusion
	return address; 
}

int RAMLRUEvict(){

// If an empty RAM slot was not find then, run the page evict algorithm to evict a page from RAM.
	usleep(0.01*1000000);	// Sleep for 0.01 s because it is evicting from RAM

	// Run the LRU algorithm to find the least recently used page in RAM
	int RAM_ind = LRU(RAM_Capacity);	
		
	// Check if there is room in SSD to put the evicted page from RAM
	int i;
	int SSD_slot = -1;
	for (i = 0; i < SSD_Capacity; i++){
		if (SSD[i] == -1){
			SSD_slot = i;
			break;
		}
	}

	// If we find a spot in SSD, we can swap the page
	if (SSD_slot != -1){
		printf("The page is in SSD, swap the page from RAM. The randomly chosen page is %d \n", RAM_ind);
		usleep(0.1*1000000);		// Sleep for 0.1 s because we are writing to the SSD
		SSD[SSD_slot] = RAM[RAM_ind]; 	// Transfer the stuff from RAM to SSD
	
	// We have to find the corresponding page entry in the page table
		int j = 0;
		for (j = 0; j < Page_Table_Capacity; j++){
			if ((page_table[j].physical_address == RAM_ind) && (page_table[j].mem_location == 1)){
				page_table[j].physical_address = SSD_slot;
				page_table[j].mem_location = 2;
				RAM[RAM_ind] = -1;
				return RAM_ind;
			}
		}
	}

	// If we can't find a spot in SSD, we have to evict from SSD
	else if (SSD_slot == -1) {
		int rtime = LRU(SSD_Capacity);
	// Check if there is room in the HD to put the evicted page from SSD
		int HD_slot = -1;
		int k;
		for (k = 0; k < HD_Capacity; k++){
		if (HD[k] == -1){
			HD_slot = k;
			break;
			}
		}
	
		if (HD_slot != -1){
		printf("The page is in HD, swap the page from SSD. The randomly chosen page is %d \n", rtime);
	// If we find a spot, we can swap the page
			usleep(2.5*1000000);
			HD[HD_slot] = SSD[rtime];
	// We have to find the corresponding page table entry

			int l;
			for (l = 0; l < Page_Table_Capacity; l++){
				if ((page_table[l].physical_address == rtime) && (page_table[l].mem_location == 2)){
					page_table[l].physical_address = HD_slot;
					page_table[l].mem_location = 3;
					SSD[rtime] = -1;
					break;
				}
			}

			usleep(0.1*1000000);
			SSD[rtime] = RAM[RAM_ind];
			int n;
			for (n = 0; n < Page_Table_Capacity; n++){
				if ((page_table[n].physical_address == RAM_ind) && (page_table[n].mem_location == 1)){
					page_table[n].mem_location = 2;
					page_table[n].physical_address = rtime;
					RAM[RAM_ind] = -1;
					return RAM_ind;
				}
			}
		}

	// If there is no spot in HD, print an error		
		else if (HD_slot == -1){
			printf("No space in Hard disk \n");
			return -10;
		}
	}

}

uint32_t get_value(vAddr address, int *valid){
	
	sem_wait(&value_get);		// Do a sem_wait for mutual exclusion
	
	// Local variables to keep track of the memory location and physical_address
	int temp_mem_location;
	int temp_physical_address;
	int value;

	// Check if the particular page entry is not valid
	if (page_table[address].valid == 0){
		printf("The page you are trying to access doesn't exist \n");
		valid = NULL;
		sem_post(&value_get);		// Do a sem_post for mutual exclusion
		return 0;
	}

	// Check if the address is not found
	if ((page_table[address].physical_address > 999) || (page_table[address].physical_address < 0)){
		printf("The page : %d you are trying to access doesn't exist \n", address);
		valid = NULL;
		sem_post(&value_get);		// Do a sem_post for mutual exclusion
		return 0;
	}

	// If the entry is valid, get the value at that particular index
	else {
		printf ("Getting the value at the address %d \n", address);
		temp_mem_location = page_table[address].mem_location;
		temp_physical_address = page_table[address].physical_address;

		page_table[address].referenced = mod_count;
		page_table[address].modified = mod_count;

		// Update
		int s = 0;
		for (s = 0; s < Page_Table_Capacity; s++){
			if ((s != address) && (page_table[s].valid == 1)) {
				page_table[s].referenced++;
			}
		}

		if (temp_mem_location == 1){
			*valid = 1;
			value = RAM[temp_physical_address];
			printf("The value in RAM %d \n", value);
			sem_post(&value_get);		// Do a sem_post for mutual exclusion
			return value;
		}

		else if (temp_mem_location == 2){
			*valid = 1;
			value = SSD[temp_physical_address];
			printf("The value in SSD %d \n", value);
			sem_post(&value_get);		// Do a sem_post for mutual exclusion
			return value;
		}

		else if (temp_mem_location == 3){
			*valid = 1;
			value = HD[temp_physical_address];
			printf("The value in HD %d \n", value);
			sem_post(&value_get);		// Do a sem_post for mutual exclusion
			return value;
		}

		// If the address was not found in the page table
		else {
			valid = NULL;
			sem_post(&value_get);		// Do a sem_post for mutual exclusion
			return 0;
		}
	}		
}

void store_value (vAddr address, uint32_t *value){

	sem_wait(&value_store);		// Do a sem_wait for mutual exclusion

	// If the page table entry was not found
	if (page_table[address].valid == 0){
		printf("The page you are trying to access doesn't exist \n");
	}

	// Check if the address is not found
	if ((page_table[address].physical_address > 999) || (page_table[address].physical_address < 0)){
		printf("The page : %d you are trying to access doesn't exist \n", address);
	}

	// Check if the page is in RAM
	else if (page_table[address].mem_location == 1){
		usleep(0.01*1000000);	// We are writing to the RAM
		printf("Changing the value of the page in the RAM %d \n", page_table[address].physical_address);
		RAM[page_table[address].physical_address] = *value;
		printf("The value in the RAM has been changed to %d \n", *value);
		page_table[address].referenced = mod_count;
		page_table[address].modified = mod_count;

		// Update
		int s = 0;
		for (s = 0; s < Page_Table_Capacity; s++){
			if ((s != address) && (page_table[s].valid == 1)) {
				page_table[s].referenced++;
			}
		}
	}

	// Check if the page is in SSD then swap the page from RAM
	else if (page_table[address].mem_location == 2){
		usleep(0.1*1000000);	// Sleep for 0.1 s as we are evicting a page from SSD
		printf("The page is in SSD, swap it from the RAM %d \n", page_table[address].physical_address);
		page_table[address].referenced = mod_count;
		page_table[address].modified = mod_count;

		// Update
		int s = 0;
		for (s = 0; s < Page_Table_Capacity; s++){
			if ((s != address) && (page_table[s].valid == 1)) {
				page_table[s].referenced++;
			}
		}
		// Check if there is a spot in RAM to store the value
		int RAM_slot = -1;
		int i;
		for (i = 0; i < RAM_Capacity; i++){
			if (RAM[i] == -1){
				RAM_slot = i;
				break;
			}
		}
	
		// There was a spot in RAM, so store the value in RAM
		if (RAM_slot != -1){
			printf("The value was stored in RAM which was in SSD \n");
			RAM[RAM_slot] = *value;
			page_table[address].mem_location = 1;
			page_table[address].physical_address = RAM_slot;
		}

		// If there was no spot, then we have to run the page eviction algo
		else if (RAM_slot == 1){
			RAM_slot = RAMLRUEvict();
			printf("The value was stored in RAM which was in SSD \n");
			RAM[RAM_slot] = *value;
			page_table[address].mem_location = 1;
			page_table[address].physical_address = RAM_slot;
		}
	}

	// Check if the page is in HD then swap the page into SSD and then RAM
	else if (page_table[address].mem_location == 3){
		usleep(2.50*1000000);	// Sleep for 2.50 s as we reading from disk
		printf("The page is in HD, swap it from the SSD %d \n", page_table[address].physical_address);
		page_table[address].referenced = mod_count;
		page_table[address].modified = mod_count;

		// Update
		int s = 0;
		for (s = 0; s < Page_Table_Capacity; s++){
			if ((s != address) && (page_table[s].valid == 1)) {
				page_table[s].referenced++;
			}
		}
		
		// Check if there is an empty slot in SSD
		int SSD_slot = -1;
		int i;
		for (i = 0; i < SSD_Capacity; i++){
			if (SSD[i] = -1){
				SSD_slot = i;
				break;
			}
		}
	
		// There was an empty spot in the SSD
		if (SSD_slot != -1){
			printf("The value was stored in SSD which was in HD \n");
			SSD[SSD_slot] = *value;
			page_table[address].mem_location = 2;
			page_table[address].physical_address = SSD_slot;

			// Now, we have to see if there is an empty slot in RAM
			if (page_table[address].mem_location == 2){
				usleep(0.1*1000000);	// Sleep for 0.1 s as we are evicting a page from SSD
				printf("The page is in SSD, swap it from the RAM %d \n", page_table[address].physical_address);
		// Check if there is a spot in RAM to store the value
				int RAM_slot = -1;
				int i;
				for (i = 0; i < RAM_Capacity; i++){
					if (RAM[i] == -1){
						RAM_slot = i;
						break;
					}
				}
	
		// There was a spot in RAM, so store the value in RAM
				if (RAM_slot != -1){
					printf("The value was stored in RAM which was in SSD \n");
					RAM[RAM_slot] = *value;
					page_table[address].mem_location = 1;
					page_table[address].physical_address = RAM_slot;
				}

		// If there was no spot, then we have to run the page eviction algo
				else if (RAM_slot == 1){
					RAM_slot = RAMLRUEvict();
					printf("The value was stored in RAM which was in SSD \n");
					RAM[RAM_slot] = *value;
					page_table[address].mem_location = 1;
					page_table[address].physical_address = RAM_slot;
				}
			}
		}
		// There was no empty slot in the SSD
		else if (SSD_slot == -1){
			
		// Evict a page from the SSD to HD
			// Check if there is an empty slot in HD
			int rtime = LRU(SSD_Capacity);
			int HD_slot;
			int i;
			for (i = 0; i < HD_Capacity; i++){
				if (HD[i] == -1){
					HD_slot = i;
					break;
				}
			}

			// Check if there is an empty slot in HD

			if (HD_slot != -1){

	// If we find a spot, we can swap the page
				usleep(2.5*1000000);
				HD[HD_slot] = SSD[rtime];
	// We have to find the corresponding page table entry

				int l;
				for (l = 0; l < Page_Table_Capacity; l++){
					if ((page_table[l].physical_address == rtime) && (page_table[l].mem_location == 2)){
						page_table[l].physical_address = HD_slot;
						page_table[l].mem_location = 3;
						SSD[rtime] = -1;
						break;
					}
				}
			}
			// If there is no spot in HD, print an error		
			else if (HD_slot == -1){
				printf("No space in Hard disk, we can't store the value \n");
			}
		
			SSD[rtime] = *value;
			page_table[address].mem_location = 2;
			page_table[address].physical_address = rtime;
			
			// Now run the algorithm to bring the page to RAM
			usleep(0.1*1000000);	// Sleep for 0.1 s as we are evicting a page from SSD
			printf("The page is in SSD, swap it from the RAM %d \n", page_table[address].physical_address);
		// Check if there is a spot in RAM to store the value
			int RAM_slot = -1;
			int d;
			for (d = 0; d < RAM_Capacity; d++){
				if (RAM[d] == -1){
					RAM_slot = d;
					break;
				}
			}
	
		// There was a spot in RAM, so store the value in RAM
			if (RAM_slot != -1){
				printf("The value was stored in RAM which was in SSD \n");
				RAM[RAM_slot] = *value;
				page_table[address].mem_location = 1;
				page_table[address].physical_address = RAM_slot;
			}

			// If there was no spot, then we have to run the page eviction algo
			else if (RAM_slot == 1){
				RAM_slot = RAMLRUEvict();
				printf("The value was stored in RAM which was in SSD \n");
				RAM[RAM_slot] = *value;
				page_table[address].mem_location = 1;
				page_table[address].physical_address = RAM_slot;
			}
		}			
	}
	else {
		printf("We encountered an error while storing the value");
	}

	sem_post(&value_store);		// Do a sem_post for mutual exclusion							
}

void free_page (vAddr address){
	
	sem_wait(&page_free);		// Do a sem_wait for mutual exclusion

	// Check if the page is in RAM, then free the page
	if (page_table[address].mem_location == 1){
		RAM[page_table[address].physical_address] = -1;
	}
	
	// Check if the page is in SSD, then free the page
	else if (page_table[address].mem_location == 2){
		SSD[page_table[address].physical_address] = -1;
	}

	// Check if the page is in HDD, then free the page
	else if (page_table[address].mem_location == 3){
		HD[page_table[address].physical_address] = -1;
	}

	// Also empty the page table entry
	page_table[address].physical_address = -1;
	page_table[address].mem_location = 0;
	page_table[address].modified = 0;
	page_table[address].referenced = 0;
	page_table[address].valid = 0;
	page_table[address].flag = 0;

	sem_post(&page_free);		// Do a sem_post for mutual exclusion
}
			
void memoryMaxer() {
	vAddr indexes[1000];
	int i = 0;
	for (i = 0; i < 1000; ++i) {
		indexes[i] = create_page();
		printf("Page created %d \n", indexes[i]);
		int valid = 0;
		int value = get_value(indexes[i], &valid);
		value = (i * 3);
		store_value(indexes[i], &value);
	}
	
	for (i = 0; i < 1000; ++i) {
		free_page(indexes[i]);
	}
}

int LRU(int capacity){

	int modind = 0;
	int pgind = 0;
	int ref_count;
	int REF_count[capacity];
	int REF_ind[capacity];
	int page_number[capacity];

	if (capacity == RAM_Capacity){
		for (modind = 0; modind < capacity; modind++){
			for (pgind = 0; pgind < Page_Table_Capacity; pgind++){
				if ((page_table[pgind].physical_address == modind) && (page_table[pgind].mem_location == 1) && (page_table[pgind].valid == 1)){
					REF_count[modind] = page_table[pgind].referenced;
					REF_ind[modind] = modind;
					page_number[modind] = page_table[pgind].virtual_address;
				}
			}
		}
	}

	if (capacity == SSD_Capacity){
		for (modind = 0; modind < capacity; modind++){
			for (pgind = 0; pgind < Page_Table_Capacity; pgind++){
				if ((page_table[pgind].physical_address == modind) && (page_table[pgind].mem_location == 2) && (page_table[pgind].valid == 1)){
					REF_count[modind] = page_table[pgind].referenced;
					REF_ind[modind] = modind;
					page_number[modind] = page_table[pgind].virtual_address;
				}
			}
		}
	}

	// Find the minimum element in the REf_count array
	int minimum = REF_count[0];
	int RAM_ind = REF_ind[0];
	int current = page_number[0];
	int minind;
	for (minind = 0; minind < capacity; minind++){
		if (REF_count[minind] > minimum){
			minimum = REF_count[minind];
			RAM_ind = REF_ind[minind];
			current = page_number[minind];
		}
	}
	
	printf("The chosen page based on LRU algorithm from RAM/SSD is %d \n", RAM_ind);	
	return RAM_ind;					
}

void testmethod(){

	vAddr indexes[1000];
	int i = 0;
	for (i = 0; i < 1000; ++i) {
		indexes[i] = create_page();
		printf("Page created %d \n", indexes[i]);
		int valid = 0;
		int value = get_value(1000 - (i + 12), &valid);
		value = (i * 3);
		store_value(indexes[i], &value);
	}
	
	for (i = 0; i < 1000; ++i) {
		free_page(indexes[i]);
	}

}

void testMethod2() {
	vAddr indexes[1000];
	int i = 0;
	for( i = 0; i < 1000; ++i ){
		indexes[i] = create_page();
		printf("Page created %d \n", i);
	}	
	for(i = 0; i < 1000; i++) {
		int value = i * 2;
		store_value(indexes[i], &value);
	}
	for(i = 0; i < 1000; i++)
		free_page(indexes[i]);
}

// Initialize the threads; 10 threads are created in this function
void initThreads(){
	int i; int error; int j = 0;
	
	// Store the thread number in the threads_count array
	for (i = 0; i < 10; i++){
		threads_count[i] = i;
	}
	
	// Use the pthread_create function to the create the threads
	for (i = 0; i < 10; i++){
		error = pthread_create(&tid[i], NULL, thread_process, &threads_count[i]);
		if (error != 0){
			printf("\ncan't create thread :[%s]", strerror(error));		// Throw an error if the thread can't get created
		}
	}

	// Join the threads after the threads are created
	while (j < 10){
		pthread_join(tid[j], NULL);
		j++;
	}
}

// Thread_process function for each thread
void *thread_process(void *arg){
	int i;
	int value;
	int valid;
	vAddr indexes[1000];
	int * threads_count = (int*) arg;

	while (1){
		// Create 145 threads to create the multi-threaded program
		for (i = 0; i < 1000; i++){
			indexes[i] = create_page();	// Use the create page function to create the pages
			sem_wait(&lock_page);		// Use the semaphore to lock / mutual exclusion
			printf("Thread number : %d, is opening address %d \n", (*threads_count), i);
			valid = 0;
			value = get_value(indexes[i], &valid);	// Get the value stored using the get_value() function
			printf("Value at the specified index is %d \n", value);
			
			printf("Thread number : %d, is closing address %d \n", (*threads_count), i);
			sem_post(&lock_page);			// Do a post on the sempahore
		}
	}
}
