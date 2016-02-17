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
void parentProcess(int backgroundstate, int rval, char* command);    	// This function does the parent process after the fork function is returned. It is called in main().
void childProcess(int argc, char *argv[]);	// This function does the child process after the fork function is returned. It is called in 							   main().
long gettimeinms();		// This function converts the wall-clock time in milliseconds.
char *readArguments(char userinput);

int MAX_CHARACTERS = 129;
int MAX_ARGUMENTS = 33;
#define Array_Size 129

typedef struct {
	char* command;
	int statusjob;
	int pid;
	long starttime;
}bgjob;

bgjob backgroundjobs[Array_Size]; 
void backgroundprocesses();

int position = 0;

// Main method with the function calls
int main(int argc, char *argv[]) {
	
	// Continuous while loop 
	while (1) {
		
		printf("==> "); 	// Prompt characters

		// Check the user input
		char userinput[MAX_CHARACTERS]; 	// Set the maximum character limit for the user input on the command line	
		
		// This if statement checks the userinput and throws an error if it is empty or an error has occurred
		if (fgets(userinput, MAX_CHARACTERS, stdin) == NULL) {
			printf("The end of file has been reached or an error has occurred \n");	// Throw an error
			exit(1);	// Exit the program
		}
		
		// This if statement checks if the user input exceeded the maximum number of characters allowed
		if (strlen(userinput) > MAX_CHARACTERS) {
			printf("The maximum number of characters has been exceeded \n");	// Throw an error if the condition is not met
			exit(1);	// Exit the program
		}

		userinput[strlen(userinput) - 1] = '\0';	// Replace the new line character with the null terminator
	
		//ar *arguments = readArguments(userinput);			// Read the arguments after tokenizing to build the array of strings
	
		char **arguments = (char**)malloc(MAX_ARGUMENTS * sizeof(char*));
		char* token = strtok(userinput, " ");
	
		int j; 
		
		int count = 0;
		for (j = 0; j < MAX_ARGUMENTS && token != NULL; j++) {
			*(arguments + j) = (char*)malloc(sizeof(char) * strlen(token));
			strcpy(*(arguments + j), token);
			token = strtok(NULL, " ");
			count++;
		}

		if (arguments[MAX_ARGUMENTS - 1] != NULL) {
			printf("%s \n", arguments[MAX_ARGUMENTS - 1]);
			printf("The number of arguments provided exceeds the maximum number \n");
			continue;
		}
		
		char* command = arguments[0];
		char* lastcommand = arguments[count - 1];

		if (strcmp(command, "exit") == 0) {
			int m;
			for (m = 0; m <= position; m++) {
				if((backgroundjobs[m].statusjob) == 1){
					printf ("There are still background processes running, please wait! \n");
				}
			}
			printf("It was great having you! :) \n");
			exit(1);
		}

		if (strcmp(command, "cd") == 0) {
			chdir(arguments[1]);
			continue;
		}
					
		int backgroundstate = 0;

		if (strcmp(lastcommand, "&") == 0){			
			backgroundstate = 1;
			arguments[count - 1] = NULL;
			count--;
		}

		if (strcmp(command, "jobs") == 0) {
			printf("The current jobs running are: \n");

			int k, l;
			for(k = 0; k <= position; k++){
				if((backgroundjobs[k].statusjob) == 1) {
					printf("[%i] %i %s\n", position, backgroundjobs[k].pid, backgroundjobs[k].command);
				}
			}
			for(l = 0; l <= position; l++){
				if((backgroundjobs[l].statusjob) == 0) {
					printf("[%i] %i %s completed.\n", position, backgroundjobs[l].pid, backgroundjobs[l].command);
				}
			}
			continue;
		}			
		
		int rval;  		// Variable to keep track of the fork() return value
	
		// This if statement checks the command line arguments, treating the first argument as a program to be executed. If the number of arguments are not valid enough, the code is exited.
		if (argc < 1) {  // This statement checks whether the number of input arguments were less than 2, as that would not be enough arguments
			printf("The command provided is illegal \n");  // print an error message
			exit(1); // Exit the program early due to not enough number of input arguments
		}

		rval = fork(); 		// Get the return value of the fork function
		
		// This if-else statement checks if we are in the child process or the parent process
		if (rval != 0) {
			parentProcess(backgroundstate, rval, command);	// Calls the parent process function if the return value from the fork function is not 0
		}
	
		else {
			int result = execvp(command, arguments);
			if (result == -1) {
				printf("Invalid Command \n");
				continue;
			}	
		}

	}
	return 0;
}

// In the parent process, wait for the completion and then print statistics about the child process's execution.
void parentProcess(int backgroundstate, int rval, char* command) {
	
	// Local variables defined
	long begin_time; long end_time; struct rusage usage; int status; long wall_clock_time; long user_time; long system_time;
	
	backgroundprocesses();

	if (backgroundstate == 1) {
		begin_time = gettimeinms();
		backgroundjobs[position].pid = rval;
		backgroundjobs[position].starttime = begin_time;
		backgroundjobs[position].statusjob = 1;
		backgroundjobs[position].command = command;
		position++;  
	}
	
	else {
		begin_time = gettimeinms();
		//getrusage(RUSAGE_CHILDREN, &usage);
		wait4(rval, &status, 0, &usage);
		//getrusage(RUSAGE_CHILDREN, &usage);

		end_time = gettimeinms();	// Get the end wall_clock time in milliseconds by calling the gettimeinms() function
	
		if (status != 0) {		// Check the status of the child process in the call
			exit(1);		// Exit if the status is not 0
		}

		wall_clock_time = end_time - begin_time;	// Calculate the total wall clock time
		user_time = (usage.ru_utime.tv_sec * 1000) + (usage.ru_utime.tv_usec / 1000);	// Get the CPU user time in milliseconds
		system_time = (usage.ru_stime.tv_sec * 1000) + (usage.ru_stime.tv_usec / 1000);	// Get the CPU system time in milliseconds
	
	
	//else {
		printf("Elapsed wall-clock time in milliseconds is %ld ms\n", wall_clock_time);	// Print the elapsed wall-clock time on the shell
		printf("Amount of CPU user time in milliseconds is %ld ms\n", user_time);
		printf("Amount of CPU system time in milliseconds is %ld ms\n", system_time);	
		printf("Number of times the process was preempted involuntarily is %ld\n", usage.ru_nivcsw);
		printf("Number of times the process gave up the CPU voluntarily is %ld\n", usage.ru_nvcsw);
		printf("Number of page faults are %ld\n", usage.ru_majflt);
		printf("Number of page faults that could be satisfied using unreclaimed pages are %ld\n", usage.ru_minflt);
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

void backgroundprocesses() {
	long begin_time; long end_time; int status; long wall_clock_time; long user_time; long system_time;
	struct rusage usagedata;
	int pid = wait3(&status, WNOHANG, &usagedata);
	while(pid != 0 && pid != -1){
		int j = 0;
		for (j = 0; j <= position; j++) {
			if(backgroundjobs[j].pid == pid){
				backgroundjobs[j].statusjob = 0;  // finished
				begin_time = backgroundjobs[j].starttime;
				end_time = gettimeinms();
				
				wall_clock_time = end_time - begin_time;	// Calculate the total wall clock time
				user_time = (usagedata.ru_utime.tv_sec * 1000) + (usagedata.ru_utime.tv_usec / 1000);	// Get the CPU user time in milliseconds

				system_time = (usagedata.ru_stime.tv_sec * 1000) + (usagedata.ru_stime.tv_usec / 1000);	// Get the CPU system time in milliseconds
				
				printf("Elapsed wall-clock time in milliseconds is %ld ms\n", wall_clock_time);	// Print the elapsed wall-clock time on the shell
				printf("Amount of CPU user time in milliseconds is %ld ms\n", user_time);
				printf("Amount of CPU system time in milliseconds is %ld ms\n", system_time);	
				printf("Number of times the process was preempted involuntarily is %ld\n", usagedata.ru_nivcsw);
				printf("Number of times the process gave up the CPU voluntarily is %ld\n", usagedata.ru_nvcsw);
				printf("Number of page faults are %ld\n", usagedata.ru_majflt);
				printf("Number of page faults that could be satisfied using unreclaimed pages are %ld\n", usagedata.ru_minflt);

			}
		}
	pid = wait3(NULL, WNOHANG, &usagedata);
	}
}

	
	
	
