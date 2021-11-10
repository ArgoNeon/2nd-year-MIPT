#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define DECL 1
#define START 2
#define WAND 3
#define FINISH 4
#define RES 5
#define GOODBYE 6
#define JUDGEARR 7

struct msgbuf {
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

int runner(int n, int i, int id) {
  const char snd_err[30] = "msgsnd error(runner)";
  const char rcv_err[30] = "msgrcv error(runner)";
  struct msgbuf buf;
  int snd;
  ssize_t rcv;

  printf("The participant %d came.\n", i);
  rcv = msgrcv(id, &buf, 0, JUDGEARR, MSG_NOERROR);
  check_err(rcv, rcv_err);
  printf("The participant %d declared himself to the judge.\n", i);
  buf.mtype = DECL;
  snd = msgsnd(id, &buf, 0, MSG_NOERROR);
  check_err(snd, snd_err);
  printf("The participant %d is waiting for the start of the competition ...\n",
         i);
  rcv = msgrcv(id, &buf, 0, START, MSG_NOERROR);
  check_err(rcv, rcv_err);
  printf("The participant %d is waiting for the wand...\n", i);
  rcv = msgrcv(id, &buf, 0, WAND, MSG_NOERROR);
  check_err(rcv, rcv_err);
  printf("The participant %d ran.\n", i);
  printf("The participant %d handed over the wand.\n", i);
  buf.mtype = WAND;
  snd = msgsnd(id, &buf, 0, MSG_NOERROR);
  check_err(snd, snd_err);
  buf.mtype = FINISH;
  snd = msgsnd(id, &buf, 0, MSG_NOERROR);
  check_err(snd, snd_err);
  printf("The participant %d is waiting for the results...\n", i);
  rcv = msgrcv(id, &buf, 0, RES, MSG_NOERROR);
  check_err(rcv, rcv_err);
  printf("The participant %d said goodbye.\n", i);
  buf.mtype = GOODBYE;
  snd = msgsnd(id, &buf, 0, MSG_NOERROR);
  check_err(snd, snd_err);
  return 0;
}

int judge(int n, int id) {
  const char snd_err[30] = "msgsnd error(judge)";
  const char rcv_err[30] = "msgrcv error(judge)";
  struct msgbuf buf;
  int i, snd;
  ssize_t rcv;

  printf("The judge came.\n");
  buf.mtype = JUDGEARR;
  for (i = 1; i <= n; i++) {
    snd = msgsnd(id, &buf, 0, MSG_NOERROR);
    check_err(snd, snd_err);
  }
  printf("The judge is waiting for the participants of the competition...\n");
  for (i = 1; i <= n; i++) {
    rcv = msgrcv(id, &buf, 0, DECL, MSG_NOERROR);
    check_err(rcv, rcv_err);
  }
  printf("All participants showed up to the judge.\n");
  printf("The judge announced the start of the competition.\n");
  buf.mtype = START;
  for (i = 1; i <= n; i++) {
    snd = msgsnd(id, &buf, 0, MSG_NOERROR);
    check_err(snd, snd_err);
  }
  printf("Command start to the first participant.\n");
  buf.mtype = WAND;
  snd = msgsnd(id, &buf, 0, MSG_NOERROR);
  check_err(snd, snd_err);
  for (i = 1; i <= n; i++) {
    rcv = msgrcv(id, &buf, 0, FINISH, MSG_NOERROR);
    check_err(rcv, rcv_err);
  }
  printf("The last participant finished.\n");
  printf("The judge announces the results.\n");
  buf.mtype = RES;
  for (i = 1; i <= n; i++) {
    snd = msgsnd(id, &buf, 0, MSG_NOERROR);
    check_err(snd, snd_err);
  }
  for (i = 1; i <= n; i++) {
    rcv = msgrcv(id, &buf, 0, GOODBYE, MSG_NOERROR);
    check_err(rcv, rcv_err);
  }
  printf("The judge announces the end of the competition.\n");
  return 0;
}

int main(int argn, const char* argv[]) {
  const char get_err[20] = "msgget error\n";
  const char ctl_err[20] = "msgctl error\n";
  int n, i, id, ctl;
  key_t key;
  pid_t pid;

  if (argn == 2) {
    n = atoi(argv[1]);

    key = ftok("run", 0);

    id = msgget(key, IPC_CREAT | 0777);

    if (id < 0) {
      perror(get_err);
      errno;
      return -1;
    }

    printf("key: %d\nid: %d\n", key, id);

    for (i = 1; i <= n; i++) {
      pid = fork();
      if (pid == 0) {
        runner(n, i, id);
        return 0;
      }
    }

    judge(n, id);
    ctl = msgctl(id, IPC_RMID, NULL);

    if (ctl < 0) {
      perror(ctl_err);
      errno;
      return -1;
    }

    return 0;
  } else {
    printf("Write only number of runners.\n");
    return -1;
  }
}
