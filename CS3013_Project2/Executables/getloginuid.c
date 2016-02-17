#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
// These values MUST match the unistd_32.h modifications:
#define __NR_cs3013_syscall1 355
#define __NR_cs3013_syscall2 356
#define __NR_cs3013_syscall3 357

long testCall3 (unsigned short *pid, unsigned short *uid) {
	return (long) syscall(__NR_cs3013_syscall3, pid, uid);
}

int main (int argc, char** argv) {

	unsigned short *pid = (unsigned short *) malloc(sizeof(unsigned short));
	unsigned short *uid = (unsigned short *) malloc(sizeof(unsigned short));	
	
	*pid = atoi(*(argv+1));

	long retvalue = testCall3(pid, uid);
	if (retvalue == 0) {
		printf("The process is owned by : %d \n", *uid);
	}
	else {
		printf("The process owner not found \n");
	}
	printf("The return value of the system call is:\n");	
	printf("\tcs3013_syscall3: %ld\n", testCall3(pid, uid));
	return 0;
}
