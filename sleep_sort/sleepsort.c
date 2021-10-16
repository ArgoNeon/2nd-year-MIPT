#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argn, char* argv[]) {
pid_t pid;
int status, i = 0;	
unsigned a;

	if (argn >= 2) {
	for (i = 1; i < argn; i++) {
	if (getpid() != 0) {
		pid = fork();
		if (pid == 0) {
			a = atoi(argv[i]);	
			usleep(1000 * a);
			printf ("%u ", a);
			break;
		}
	}
	} 
	if (pid != 0) {
		for (i = 1; i < argn; i++) {
			wait(&status);
		}
		if (WIFEXITED(status)) {
			printf("Sleepsort\n");
		}	
	}
	}
	return 0;
}
