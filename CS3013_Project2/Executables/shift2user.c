#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
// These values MUST match the unistd_32.h modifications:
#define __NR_cs3013_syscall1 355
#define __NR_cs3013_syscall2 356
#define __NR_cs3013_syscall3 357

long testCall2 (unsigned short *pid, unsigned short *uid) {
	return (long) syscall(__NR_cs3013_syscall2, pid, uid);
}

int main (int argc, char** argv) {

	unsigned short *pid = (unsigned short *) malloc(sizeof(unsigned short));
	unsigned short *uid = (unsigned short *) malloc(sizeof(unsigned short));	
	
	*pid = atoi(*(argv+1));
	*uid = atoi(*(argv+2));
	
	long retvalue = testCall2(pid,uid);
	if (retvalue == 0) {
		printf("Shit2user was successful \n");
	}
	else {
		printf("The shift2user was unsuccessful \n");
	}
	printf("The return value of the system call is:\n");	
	printf("\tcs3013_syscall2: %ld\n", testCall2(pid, uid));
	return 0;
}
