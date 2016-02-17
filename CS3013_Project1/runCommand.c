// Vishal Kumar Rathi (vkrathi@wpi.edu)
// CS 3013 - Operating Systems - Project 1

/* This program reads in its own command line arguments, treating the first argument as a program to be executed. It executes the command and pass the additional arguments as parameters to the invoked command. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>		// Required header file for getrusage() and gettimeofday() functions
#include <sys/resource.h>	// Required header file for getrusage() and gettimeofday() functions
#include <sys/types.h>		// Required header file for wait() function
#include <sys/wait.h>		// Required header file for wait() function

// Functions definitions
void parentProcess();    	// This function does the parent process after the fork function is returned. It is called in main().
void childProcess(int argc, char *argv[]);	// This function does the child process after the fork function is returned. It is called in 							   main().
long gettimeinms();		// This function converts the wall-clock time in milliseconds.

// Main method with the function calls
int main(int argc, char *argv[]) {
	
	int rval;  		// Variable to keep track of the fork() return value
	
	// This if statement checks the command line arguments, treating the first argument as a program to be executed. If the number of arguments are not valid enough, the code is exited.
	if (argc < 2) {  // This statement checks whether the number of input arguments were less than 2, as that would not be enough arguments
		printf("The command provided is illegal\n");  // print an error message
		exit(1); // Exit the program early due to not enough number of input arguments
	}

	rval = fork(); 		// Get the return value of the fork function
		
	// This if-else statement checks if we are in the child process or the parent process
	if (rval != 0) {
		parentProcess();	// Calls the parent process function if the return value from the fork function is not 0
	}
	
	else {
		childProcess(argc, argv);	// Calls the child process function if the return value from the fork function is 0	
	}

	return 0;
}

// In the parent process, wait for the completion and then print statistics about the child process's execution.
void parentProcess() {
	
	// Local variables defined
	long begin_time; long end_time; struct rusage usage; int rusage_status; int status; long wall_clock_time; long user_time; long system_time;	
	
	begin_time = gettimeinms();	// Get the start wall-clock time in milliseconds by calling the gettimeinms() function

	rusage_status = getrusage(RUSAGE_CHILDREN, &usage);	// This function returns the resource usage statistics for the calling process. It is the sume of all resources used by all threads in the process. rusage_status tells us if the process was a success.
	
	if (rusage_status == 0) {	// If the return value of getrusage is a 0, it is a success. If it is a success, get the resource usage statistics of the child process.
		wait(&status);		// Wait for the state changes in the child of the calling process
		getrusage(RUSAGE_CHILDREN, &usage);	// Get the resource usage statistics of the child process
	}

	end_time = gettimeinms();	// Get the end wall_clock time in milliseconds by calling the gettimeinms() function
	
	if (status != 0) {		// Check the status of the child process in the call
		exit(1);		// Exit if the status is not 0
	}

	wall_clock_time = end_time - begin_time;	// Calculate the total wall clock time
	user_time = (usage.ru_utime.tv_sec * 1000) + (usage.ru_utime.tv_usec / 1000);	// Get the CPU user time in milliseconds
	system_time = (usage.ru_stime.tv_sec * 1000) + (usage.ru_stime.tv_usec / 1000);	// Get the CPU system time in milliseconds

	printf("Elapsed wall-clock time in milliseconds is %ld ms\n", wall_clock_time);	// Print the elapsed wall-clock time on the shell
	printf("Amount of CPU user time in milliseconds is %ld ms\n", user_time);	// Print the CPU user time on the shell
	printf("Amount of CPU system time in milliseconds is %ld ms\n", system_time);	// Print the CPU system time on the shell
	printf("Number of times the process was preempted involuntarily is %ld\n", usage.ru_nivcsw); // Print the number of times the process was preempted involuntarily on the shell
	printf("Number of times the process gave up the CPU voluntarily is %ld\n", usage.ru_nvcsw);	// Print the number of times the process gave up the CPU voluntarily on the shell
	printf("Number of page faults are %ld\n", usage.ru_majflt);	// Print the number of page faults on the shell
	printf("Number of page faults that could be satisfied using unreclaimed pages are %ld\n", usage.ru_minflt);	// Print the number of page faults that could be satisfied using reclaimed pages on the shell
				
}

// This function takes care of the child process
void childProcess(int argc, char *argv[]) {

	char *elements[argc]; int i;	// Local variables defined
	
	// This loop goes through argv[] and puts the contents starting at index position 1 into an array
	for (i = 1; i <= (argc - 1); i++) {
		elements[i - 1] = argv[i];	// Putting the contents of argv[] into elements array
	}
		
	elements[argc - 1] = NULL;	// Setting the last index position of the elements array to NULL
	
	if (execvp(elements[0], elements) == -1) {	// Running the execvp function to check for the command
		printf("Unknown or Bad command\n");	// Print a message if it is an unknown or bad command
		exit(1);				// Exit the process if the command provided is bad or unknown
	}	
}

// This function returns wall-clock time in milli-seconds
long gettimeinms() {

	long total_time;	// Local variable to store the time
	struct timeval tv;	// Defining the struct tv 
	gettimeofday(&tv, NULL);	// Calling the gettimeofday() function to the get the time
	
	total_time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);	// Converting the time in milliseconds

	return total_time;	// return the wall-clock time to be use further
}
	
	
	
