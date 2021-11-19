#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#define DO 1
#define DONE 2
#define NEXT 3
#define NUM 2

struct msgbuf{
	long mtype;
	char mtext[0];
};

void check_err(int x, const char* name) {
	if (x < 0) {
		perror(name);
		errno;
		exit(1);
	}
}

int screw(int id1, int id2, int n) {
	const char snd_err[30] = "msgsnd error(screw)";
	const char rcv_err[30] = "msgrcv error(screw)";
	struct msgbuf buf;
	int snd, i, j;
	ssize_t rcv;
	
	printf("Start work\n");
	buf.mtype = NEXT;
	snd = msgsnd(id1, &buf, 0, MSG_NOERROR);
	check_err(snd,snd_err);

	for (i = 0; i < n; i++) {
		rcv = msgrcv(id1, &buf, 0, NEXT, MSG_NOERROR);
		check_err(rcv, rcv_err);
		printf("The worker fixed the new workpiece.\n");
		buf.mtype = DO;
		for (j = 0; j < NUM; j++) {
			snd = msgsnd(id2, &buf, 0, MSG_NOERROR);
			check_err(snd, snd_err);
		}
		printf("The worker tightened the screw.\n");
		for (j = 0; j < NUM; j++) {
			rcv = msgrcv(id1, &buf, 0, DONE, MSG_NOERROR);
			check_err(rcv, rcv_err);
		}
		printf("The worker removed the new part.\n");
		buf.mtype = NEXT;
		snd = msgsnd(id1, &buf, 0, MSG_NOERROR);
		check_err(snd,snd_err);
	}
	printf("End of work\n");
	return 0;
}

int nut(int id1, int id2) {
	const char snd_err[30] = "msgsnd error(nut)";
	const char rcv_err[30] = "msgrcv error(nut)";
	struct msgbuf buf;
	int snd;
	ssize_t rcv;

	while(1) {
		rcv = msgrcv(id2, &buf, 0, DO, MSG_NOERROR);
		if ((rcv == -1) && (errno == EIDRM))
			return 0;
		check_err(rcv, rcv_err);
		printf("The worker tightened the nut.\n");
		buf.mtype = DONE;
		snd = msgsnd(id1, &buf, 0, MSG_NOERROR);
		check_err(snd, snd_err);
	}
}

int main(int argn, char* argv[]){
	const char get_err[20] = "msgget error\n";
	const char ctl_err[20] = "msgctl error\n";
	int n, i, id1, id2, ctl;
	pid_t pid;
	
	if (argn == 2) {
		n = atoi(argv[1]);
		id1 = msgget(IPC_PRIVATE, IPC_CREAT | 0777);
		check_err(id1, get_err);
		id2 = msgget(IPC_PRIVATE, IPC_CREAT | 0777);
		check_err(id2, get_err);	
		
		for(i = 0; i < NUM; i++) {
			pid = fork();
			if(pid == 0) {
				nut(id1, id2);
				return 0;
			}
		}
		
		screw(id1, id2, n);	
		ctl = msgctl(id1, IPC_RMID, NULL);
		check_err(ctl, ctl_err);
		ctl = msgctl(id2, IPC_RMID, NULL);
		check_err(ctl, ctl_err);
		return 0;
	}else{
		printf("Enter only the number of details.\n");
		return -1;
	}
}
