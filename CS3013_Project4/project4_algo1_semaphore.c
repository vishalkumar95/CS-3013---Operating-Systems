// Vishal Rathi

// This file implements the random eviction algorithm.

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<stdint.h>
#include<semaphore.h>
#include<sys/time.h>

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
void memoryMaxer();
int RAMrandomEvict();
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

// Page_entry structure
typedef struct {
	int modified;	// modified bit
	int referenced;	// refernce bit
	int mem_location;	// Memory location; 1 for RAM, 2 for SSD, 4 for HD
	vAddr virtual_address;	// Virtual address of the entry
	int physical_address;	// Index of the entry
	int valid;		// Valid bit
	int flag;		// Flag for LIFO algorithm
}page_table_entry;

uint32_t SSD[SSD_Capacity];	// SSD_array
uint32_t HD[HD_Capacity];	// HD_array
uint32_t RAM[RAM_Capacity];	// RAM_array

// Page table
page_table_entry page_table[Page_Table_Capacity];

// Main function
int main(){

	int i;

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

// This function initializes the RAM array. This helps us keep track of the empty slots in RAM.
void setRAM(){
	int i;
	for (i = 0; i < RAM_Capacity; i++){
		RAM[i] = -1;
	}
}

// This function initializes the SSD array. This helps us keep track of the empty slots in SSD.
void setSSD(){
	int i;
	for (i = 0; i < SSD_Capacity; i++){
		SSD[i] = -1;
	}
}

// This function initializes the HD array. This helps us keep track of the empty slots in HD.
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
		return -1;
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
		RAM_slot = RAMrandomEvict();	// Random page eviction algorithm
		if (RAM_slot == -10){		// If the hard disk is full as well then, throw an error
			printf("We can not accomodate the page \n");
			sem_post(&page_create);
			return address;
		}
	}
	
	usleep(0.01*1000000);		// Sleeping since we are writing in the RAM
	RAM[RAM_slot] = 4;		// Store an initial value in the RAM
	page_table[address].mem_location = 1;	// Change the memory location
	page_table[address].valid = 1;		// Valid page
	page_table[address].physical_address = RAM_slot;	// Change the physical address which is index
	page_table[address].modified = mod_count;		// Set the modified bit
	page_table[address].referenced = mod_count;		// Set the reference bit
	page_table[address].flag = 1;

	// Update the page table flag values; this will help me with the LIFO page replacement algorithm
	int s = 0;
	for (s = 0; s < Page_Table_Capacity; s++){
		if ((page_table[s].valid == 1) && (s != address)) {
			page_table[s].flag++;
		}
	}
	
	sem_post(&page_create);	// Do the sem_post()
	return address; 	// Return the particular address
}

// This function is the random page eviction algorithm. It takes care of the swaps from SSD to RAM and also HD to SSD.
int RAMrandomEvict(){

	// If an empty RAM slot was not find then, run the page evict algorithm to evict a page from RAM.
	usleep(0.01*1000000);	// Sleep for 0.01 s because it is evicting from RAM
	int RAM_ind = rand() % RAM_Capacity;  // Choose a random page to evict.
		
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
		int rtime = rand() % SSD_Capacity;	// Random index for eviction from SSD to HD
	// Check if there is room in the HD to put the evicted page from SSD
		int HD_slot = -1;
		int k;
		for (k = 0; k < HD_Capacity; k++){
		if (HD[k] == -1){
			HD_slot = k;
			break;
			}
		}
		
		// There is an spot in HD	
		if (HD_slot != -1){
		printf("The page is in HD, swap the page from SSD. The randomly chosen page is %d \n", rtime);
		// If we find a spot, we can swap the page
			usleep(2.5*1000000);	// Sleep for the time to write in the hard disk
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
			// Page eviction from SSD to RAM
			usleep(0.1*1000000);	// Sleep for the time to write to SSD 
			SSD[rtime] = RAM[RAM_ind];
			int n;
			// Update the page table
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

/*This function obtains the indicated memory page from
lower levels of the hierarchy, if needed, and returns an integer pointer to the location in emulated RAM.
Returns NULL if the pointer cannot be provided (e.g., a page with the given address does not exist).*/

uint32_t get_value(vAddr address, int *valid){

	sem_wait(&value_get);	// Do sem_wait for mutual exclusion
	
	// Local variables to keep track of the memory location and physical_address
	int temp_mem_location;
	int temp_physical_address;
	int value;

	// Check if the particular page entry is not valid
	if (page_table[address].valid == 0){
		printf("The page you are trying to access doesn't exist \n");
		valid = NULL;
		sem_post(&value_get);	// Do sem_post for mutual exclusion
		return 0;
	}

	// Check if the address is not found
	if ((page_table[address].physical_address > 999) || (page_table[address].physical_address < 0)){
		printf("The page : %d you are trying to access doesn't exist \n", address);
		valid = NULL;
		sem_post(&value_get);	// Do sem_post for mutual exclusion
		return 0;
	}
	

	// If the entry is valid, get the value at that particular index
	else {
		printf ("Getting the value at the address %d \n", address);
		temp_mem_location = page_table[address].mem_location;
		temp_physical_address = page_table[address].physical_address;

		page_table[address].referenced = mod_count++;
		page_table[address].modified = mod_count++;

		// Check if the page was in RAM
		if (temp_mem_location == 1){
			*valid = 1;
			value = RAM[temp_physical_address];
			printf("The value in RAM %d \n", value);
			sem_post(&value_get);	// Do sem_post for mutual exclusion
			return value;
		}

		// Check if the page is in SSD
		else if (temp_mem_location == 2){
			*valid = 1;
			value = SSD[temp_physical_address];
			printf("The value in SSD %d \n", value);
			sem_post(&value_get);	// Do sem_post for mutual exclusion
			return value;
		}

		// Check if the page is in HD
		else if (temp_mem_location == 3){
			*valid = 1;
			value = HD[temp_physical_address];
			printf("The value in HD %d \n", value);
			sem_post(&value_get);	// Do sem_post for mutual exclusion
			return value;
		}

		// If the address was not found in the page table
		else {
			valid = NULL;
			sem_post(&value_get); // Do sem_post for mutual exclusion
			return 0;
		}
	}		
}

/*When the user wants to update the con-
tents of a page, the user indicates the value that should be stored in that page. If the page is in
memory, the value is written. If the page is not in RAM, the page is brought into RAM, evicting other
pages as needed, before updating the page in the RAM location.*/

void store_value (vAddr address, uint32_t *value){

	sem_wait(&value_store);		// Do sem_wait for mutual exclusion

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
		page_table[address].modified = mod_count++;
		page_table[address].referenced = mod_count++;
	}

	// Check if the page is in SSD then swap the page from RAM
	else if (page_table[address].mem_location == 2){
		usleep(0.1*1000000);	// Sleep for 0.1 s as we are evicting a page from SSD
		printf("The page is in SSD, swap it from the RAM %d \n", page_table[address].physical_address);
		page_table[address].modified = mod_count++;
		page_table[address].referenced = mod_count++;
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
			printf("The value was stored in RAM which originally was in SSD \n");
			RAM[RAM_slot] = *value;
			page_table[address].mem_location = 1;
			page_table[address].physical_address = RAM_slot;
		}

		// If there was no spot, then we have to run the page eviction algorithm
		else if (RAM_slot == 1){
			RAM_slot = RAMrandomEvict();
			printf("The value was stored in RAM which originally was in SSD \n");
			RAM[RAM_slot] = *value;
			page_table[address].mem_location = 1;
			page_table[address].physical_address = RAM_slot;
		}
	}

	// Check if the page is in HD then swap the page into SSD and then RAM
	else if (page_table[address].mem_location == 3){
		usleep(2.50*1000000);	// Sleep for 2.50 s as we reading from disk
		printf("The page is in HD, swap it from the SSD %d \n", page_table[address].physical_address);
		page_table[address].modified = mod_count++;
		page_table[address].referenced = mod_count++;
		
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
			printf("The value was stored in SSD which originally was in HD \n");
			SSD[SSD_slot] = *value;
			page_table[address].mem_location = 2;
			page_table[address].physical_address = SSD_slot;

			// Now, we have to see if there is an empty slot in RAM; Also we have to swap the page from SSD to RAM
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
					printf("The value was stored in RAM which originally was in SSD \n");
					RAM[RAM_slot] = *value;
					page_table[address].mem_location = 1;
					page_table[address].physical_address = RAM_slot;
				}

		// If there was no spot, then we have to run the page eviction algo
				else if (RAM_slot == 1){
					RAM_slot = RAMrandomEvict();
					printf("The value was stored in RAM which was originally in SSD \n");
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
			int rtime = rand() % SSD_Capacity;
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
				usleep(2.5*1000000);		// Sleep as we are writing in disk
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
		
			// Store the page in SSD
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
				printf("The value was stored in RAM which originally was in SSD \n");
				RAM[RAM_slot] = *value;
				page_table[address].mem_location = 1;
				page_table[address].physical_address = RAM_slot;
			}

			// If there was no spot, then we have to run the page eviction algorithm
			else if (RAM_slot == 1){
				RAM_slot = RAMrandomEvict();
				printf("The value was stored in RAM which originally was in SSD \n");
				RAM[RAM_slot] = *value;
				page_table[address].mem_location = 1;
				page_table[address].physical_address = RAM_slot;
			}
		}			
	}
	else {
		printf("We encountered an error while storing the value");
	}
	sem_post(&value_store);		// Do sem_post for mutual exclusion						
}

/*When the user is finally done with the memory page that has
been allocated, the user can free it. This frees the page, regardless of where it is in the hierarchy.*/

void free_page (vAddr address){
	
	sem_wait(&page_free);	// Do sem_wait for mutual exclusion

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

	sem_post(&page_free); 		// Do sem_post for mutual exclusion
}

// MemoryMaxer() program to test the code			
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

// Test program number # 1 to test the code; using 145 pages to fasten the process
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

// Test program number # 2 to test the code; using 145 pages to fasten the process
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






