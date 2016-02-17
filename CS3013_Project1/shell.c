// Vishal Kumar Rathi (vkrathi@wpi.edu)
// CS 3013 - Operating Systems - Project 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>		// Required header file for getrusage() and gettimeofday() functions
#include <sys/resource.h>	// Required header file for getrusage() and gettimeofday() functions
#include <sys/types.h>		// Required header file for wait() function
#include <sys/wait.h>		// Required header file for wait() function
#include <string.h>		// Required header file for strlen() function

// Functions definitions
void parentProcess();    	// This function does the parent process after the fork function is returned. It is called in main().

long gettimeinms();		// This function converts the wall-clock time in milliseconds.
char *readArguments(char userinput);

int MAX_CHARACTERS = 129;
int MAX_ARGUMENTS = 33;

// Main method with the function calls
int main(int argc, char *argv[]) {
	
	// Continuous while loop 
	while (1) {
		
		printf("==> "); 	// Prompt characters

		// Check the user input
		char userinput[MAX_CHARACTERS]; 	// Set the maximum character limit for the user input on the command line	
		
		// This if statement checks the userinput and throws an error if it is empty or an error has occurred
		if (fgets(userinput, MAX_CHARACTERS, stdin) == NULL) {
			printf("The end of file has been reached or an error has occurred");	// Throw an error
			exit(1);	// Exit the program
		}
		
		// This if statement checks if the user input exceeded the maximum number of characters allowed
		if (strlen(userinput) > MAX_CHARACTERS) {
			printf("The maximum number of characters has been exceeded");	// Throw an error if the condition is not met
			exit(1);	// Exit the program
		}

		userinput[strlen(userinput) - 1] = '\0';	// Replace the new line character with the null terminator
	
		char* arguments[MAX_ARGUMENTS];
		char* token = strtok(userinput, " \n");
	
		int j; 

		for (j = 0; j < MAX_ARGUMENTS; j++) {
			arguments[j] = token;
			token = strtok(NULL, " ");
		}

		if (arguments[MAX_ARGUMENTS - 1] != NULL) {
			printf("The number of arguments provided exceeds the maximum number");
		}
		
		char* command = arguments[0];

		if (strcmp(command, "exit") == 0) {
			printf("It was great having you! :) \n");
			exit(1);
		}

		if (strcmp(command, "cd") == 0) {
			chdir(arguments[1]);
			continue;
		}				
		
		int rval;  		// Variable to keep track of the fork() return value
	
		// This if statement checks the command line arguments, treating the first argument as a program to be executed. If the number of arguments are not valid enough, the code is exited.
		if (argc < 1) {  // This statement checks whether the number of input arguments were less than 2, as that would not be enough arguments
			printf("The command provided is illegal\n");  // print an error message
			exit(1); // Exit the program early due to not enough number of input arguments
		}

		rval = fork(); 		// Get the return value of the fork function
		
		// This if-else statement checks if we are in the child process or the parent process
		if (rval != 0) {
			parentProcess();	// Calls the parent process function if the return value from the fork function is not 0
		}
	
		else {
			//childProcess(argc, argv);	// Calls the child process function if the return value from the fork function is 0
			int result = execvp(command, arguments);
			if (result == -1) {
				printf("Invalid Command\n");
				continue;
			}	
		}

	}
	return 0;
}

// In the parent process, wait for the completion and then print statistics about the child process's execution.
void parentProcess() {
	
	// Local variables defined
	long begin_time; long end_time; struct rusage usage; int rusage_status; int status; long wall_clock_time; long user_time; long system_time; struct rusage old_statistics; long old_user_time; long old_system_time; 
	
	
	//previous_stats = 0;
	begin_time = gettimeinms();	// Get the start wall-clock time in milliseconds by calling the gettimeinms() function

	rusage_status = getrusage(RUSAGE_CHILDREN, &old_statistics);	// This function returns the resource usage statistics for the calling process. It is the sume of all resources used by all threads in the process. rusage_status tells us if the process was a success.
	
	if (rusage_status == 0) {	// If the return value of getrusage is a 0, it is a success. If it is a success, get the resource usage statistics of the child process.
		wait(&status);		// Wait for the state changes in the child of the calling process
		getrusage(RUSAGE_CHILDREN, &usage);	// Get the resource usage statistics of the child process
	}

	end_time = gettimeinms();	// Get the end wall_clock time in milliseconds by calling the gettimeinms() function
	
	if (status != 0) {		// Check the status of the child process in the call
		exit(1);			// Exit if the status is not 0
	}

	wall_clock_time = end_time - begin_time;	// Calculate the total wall clock time
	user_time = (usage.ru_utime.tv_sec * 1000) + (usage.ru_utime.tv_usec / 1000);	// Get the CPU user time in milliseconds
	old_user_time = (old_statistics.ru_utime.tv_sec * 1000) + (old_statistics.ru_utime.tv_usec / 1000);
	system_time = (usage.ru_stime.tv_sec * 1000) + (usage.ru_stime.tv_usec / 1000);	// Get the CPU system time in milliseconds
	old_system_time = (old_statistics.ru_stime.tv_sec * 1000) + (old_statistics.ru_stime.tv_usec / 1000);
	
	printf("Elapsed wall-clock time in milliseconds is %ld ms\n", wall_clock_time);	// Print the elapsed wall-clock time on the shell
	printf("Amount of CPU user time in milliseconds is %ld ms\n", user_time - old_user_time);
	printf("Amount of CPU system time in milliseconds is %ld ms\n", system_time - old_system_time);	
	printf("Number of times the process was preempted involuntarily is %ld\n", usage.ru_nivcsw - old_statistics.ru_nivcsw);
	printf("Number of times the process gave up the CPU voluntarily is %ld\n", usage.ru_nvcsw - old_statistics.ru_nvcsw);
	printf("Number of page faults are %ld\n", usage.ru_majflt - old_statistics.ru_majflt);
	printf("Number of page faults that could be satisfied using unreclaimed pages are %ld\n", usage.ru_minflt - old_statistics.ru_minflt);
				
}


// This function returns wall-clock time in milli-seconds
long gettimeinms() {

	long total_time;	// Local variable to store the time
	struct timeval tv;	// Defining the struct tv 
	gettimeofday(&tv, NULL);	// Calling the gettimeofday() function to the get the time
	
	total_time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);	// Converting the time in milliseconds

	return total_time;	// return the wall-clock time to be use further
}

	
	
	
