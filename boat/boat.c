#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/wait.h>

#define RAMP 0
#define BOAT 1
#define TRIP 2
#define SEAT 3

void check_err(int x, const char* name) {
	if (x < 0) {
		perror(name);
		errno;
		exit(1);
	}
}

int passenger(int i, int id) {
	struct sembuf seat[3] = {{TRIP, 0, 0}, {RAMP, -1, 0}, {BOAT, -1, 0}};
	struct sembuf get_up[3] = {{SEAT, 0, 0}, {RAMP, -1, 0}, {BOAT, 1, 0}};
	struct sembuf ramp[1] = {{RAMP, 1, 0}};
	int sem;

	printf("The passenger %d arrived at the pier\n", i);
	while (1) {
		sem = semop(id, seat, 3);
		if ((sem == -1) && (errno == EIDRM)) {             
                    printf("The passenger %d left the pier\n", i);
                    return 0;
                }
		printf("The passenger %d boarded the boat\n", i);
		semop(id, ramp, 1);		
		semop(id, get_up, 3);
		printf("The passenger %d got off the boat\n", i);
		semop(id, ramp, 1);
	}
}

int captain(int nboat, int ntrip, int id) {
	struct sembuf init[2] = {{BOAT, nboat, 0}, {SEAT, 1, 0}};
	struct sembuf start_trip[1] = {{TRIP, 1, 0}};
	struct sembuf end_trip[1] = {{TRIP, -1, 0}};
	struct sembuf wait_on[3] = {{BOAT, 0, 0}, {RAMP, -1, 0}, {SEAT, -1, 0}};
	struct sembuf wait_from[1] = {{BOAT, -nboat, 0}};
	struct sembuf rramp[1] = {{RAMP, -1, 0}};
	struct sembuf ramp[1] = {{RAMP, 1, 0}};	
	int i;
	
	printf("Start of trips.\n");
	semop(id, ramp, 1);
	for (i = 0; i < ntrip; i++) {
		semop(id, init, 2);
		semop(id, wait_on, 3);
		printf("All passengers boarded the boat\n");
		printf("The ramp rose.\n");
		semop(id, start_trip, 1);
		printf("The boat departed\n");
		printf("The boat arrived.\n");
		printf("The ramp went down.\n");
		semop(id, ramp, 1);
		semop(id, wait_from, 1);
		semop(id, rramp, 1);
		printf("All passengers got off the boat\n");
		semop(id, ramp, 1);
		semop(id, end_trip, 1);
	}
	semop(id, rramp, 1);
	printf("End of trips.\n");
	return 0;
}

int main(int argn, char* argv[]) {
	const char id_err[20] = "semget error";
	const char fork_err[20] = "fork error";
	const char ctl_err[20] = "semctl error";
	int npass, nboat, ntrip, id, i, ctl, status;
	pid_t pid;

	if (argn == 4) {
		npass = atoi(argv[1]);
		nboat = atoi(argv[2]);
		ntrip = atoi(argv[3]);
		
		if(npass < nboat)
			nboat = npass;

		id = semget(IPC_PRIVATE, 4, 0777);
		check_err(id, id_err);

		for (i = 1; i <= npass; i++) {
			pid = fork();
			check_err(pid, fork_err);
			if(pid == 0) {
				passenger(i, id);
				return 0;
			}
		}
		captain(nboat, ntrip, id);
		ctl = semctl(id, 0, IPC_RMID);
		check_err(ctl, ctl_err);

		for (i = 0; i < npass; i++) {
			wait(&status);
		}
		return 0;
	}else{
		printf("Write only number of passengers, seats on the boat and trips.\n");
		return -1;
	}
}
