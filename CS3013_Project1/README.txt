To run the runCommand.c / shell.c / shell2.c programs, please type:

>> make or make all

To clean the files, please type:

>> make clean 

The data structure that I used to keep track of the background programs is an array of structs. Each of these struct contains the PID, command name, job status and start time for the background job. The job status is a flag which tells us whether the job is finished or not. I maintained a flag called the backgroundstate which tells whether the job is in backround state. When the flag is one, the struct defining the backgroundjob is pushed into the array. This is the algorithm behind adding the backround jobs to the array. To remove the jobs, I maintain a flag called jobstatus which tells me whether the background job is finished or not. If it is 1 then that means that it is not finished. If it is 0, then it is finished. This is the algorithm behind the removal of background jobs from the array.

