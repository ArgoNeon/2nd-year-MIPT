#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

int main(int argn, char* argv[]) {
	struct timeval time0, time;
	pid_t pid;
	int status;

	gettimeofday(&time0, NULL);
	
	pid = fork();

	if (pid == 0) {
		execv("old/my_echo/myecho", argv);	

	}else {			
		wait(&status);
		if (WIFEXITED(status)) {
			gettimeofday(&time, NULL);
			printf ("%ld ms\n", time.tv_usec - time0.tv_usec);
		}
	}
	return 0;
}
