#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define NO_NUM 256
#define HERO 33

struct mes {
  char ch;
  int in;
};

struct msgbuf {
  long mtype;
  struct mes msg;
};

void check_err(int x, const char* name) {
  if (x < 0) {
    perror(name);
    errno;
    exit(1);
  }
}

int main(int argn, char* argv[]) {
  const char fork_err[20] = "fork error";
  const char get_err[20] = "msgget error";
  const char ctl_err[20] = "msgctl error";
  const char snd_err[20] = "msgsnd error";
  const char rcv_err[20] = "msgrcv error";
  const char shm_err[20] = "shmget error";
  int status, i, j, id_note, id_sem, shm_id_mas, shm_id_num, shm_id_x, ctl, old,
      snd, rcv, note = 0;
  char* notes;
  int *x, *num;
  struct msgbuf buf1, buf2;
  pid_t pid;

  id_note = msgget(IPC_PRIVATE, IPC_CREAT | 0777);
  check_err(id_note, get_err);
  id_sem = msgget(IPC_PRIVATE, IPC_CREAT | 0777);
  check_err(id_sem, get_err);
  shm_id_mas = shmget(IPC_PRIVATE, HERO * sizeof(char), IPC_CREAT | 0777);
  check_err(shm_id_mas, shm_err);
  notes = (char*)shmat(shm_id_mas, NULL, 0);
  shm_id_num = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0777);
  check_err(shm_id_num, shm_err);
  num = (int*)shmat(shm_id_num, NULL, 0);
  shm_id_x = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0777);
  check_err(shm_id_x, shm_err);
  x = (int*)shmat(shm_id_x, NULL, 0);

  buf1.mtype = NO_NUM;  // start init
  buf1.msg.ch = argv[1][0];
  buf1.msg.in = 1;
  notes[0] = argv[1][0];
  x[0] = argv[1][0];
  num[0] = 1;

  snd = msgsnd(id_note, &buf1, sizeof(struct mes), MSG_NOERROR);
  check_err(snd, rcv_err);

  buf2.mtype = HERO;
  snd = msgsnd(id_sem, &buf2, 0, MSG_NOERROR);
  check_err(snd, rcv_err);  // end init

  for (i = 0; i < HERO; i++) {
    pid = fork();
    check_err(pid, fork_err);

    if (pid == 0) {
      // printf("PROC %d\n", i);
      rcv = msgrcv(id_note, &buf1, sizeof(struct mes), NO_NUM, MSG_NOERROR);
      // printf("RCV:%ld %c mtext[1] %d proc %d\n", buf1.mtype, buf1.mtext[0],
      // buf1.mtext[1], i);
      if ((rcv < 0) && ((errno == EIDRM) || (errno == EINVAL))) {
        return 0;
      }
      if (x[0] == 0) {
        ctl = msgctl(id_note, IPC_RMID, NULL);
        check_err(ctl, ctl_err);
        ctl = msgctl(id_sem, IPC_RMID, NULL);
        check_err(ctl, ctl_err);
        ctl = shmctl(shm_id_mas, IPC_RMID, NULL);
        check_err(ctl, ctl_err);
        ctl = shmctl(shm_id_num, IPC_RMID, NULL);
        check_err(ctl, ctl_err);
        ctl = shmctl(shm_id_x, IPC_RMID, NULL);
        check_err(ctl, ctl_err);
        return 0;
      }

      rcv = msgrcv(id_sem, &buf2, 0, HERO, MSG_NOERROR);

      note = (int)buf1.msg.ch;

      while (1) {
        printf("[%d] %c\n", getpid(), note);

        buf1.msg.ch = argv[1][(int)buf1.msg.in];
        buf2.mtype = HERO;
        old = 0;
        x[0] = (int)argv[1][(int)buf1.msg.in];

        for (j = 0; j < num[0]; j++) {
          if (x[0] == notes[j]) {
            // printf("!num %d x[0] %d notes[%d] %d\n", num[0], x[0], j,
            // notes[j]);
            old++;
            buf1.msg.ch = (char)x[0];
            buf1.mtype = x[0];
            (int)buf1.msg.in++;
            // printf("id_note:%ld %c mtext[1] %d proc %d\n", buf1.mtype,
            // buf1.mtext[0], buf1.mtext[1], i);
            snd = msgsnd(id_sem, &buf2, 0, MSG_NOERROR);
            check_err(snd, snd_err);
            snd = msgsnd(id_note, &buf1, sizeof(struct mes), MSG_NOERROR);
            check_err(snd, snd_err);
            break;
          }
        }

        if (old == 0) {
          if (x[0] > 0) {
            num[0]++;
            notes[num[0] - 1] = x[0];
            // printf("notes [%d] x[0] %d\n", num[0] - 1, x[0]);
            buf1.msg.ch = (char)x[0];
            buf1.mtype = NO_NUM;
            (int)buf1.msg.in++;
            // printf("old = 0 : id_note:%ld %c mtext[1] %d proc %d HERO %d\n",
            // buf1.mtype, buf1.mtext[0], buf1.mtext[1], i, num[0]);
            snd = msgsnd(id_sem, &buf2, 0, MSG_NOERROR);
            check_err(snd, snd_err);
            snd = msgsnd(id_note, &buf1, sizeof(struct mes), MSG_NOERROR);
            check_err(snd, snd_err);
          } else {
            ctl = msgctl(id_note, IPC_RMID, NULL);
            check_err(ctl, ctl_err);
            ctl = msgctl(id_sem, IPC_RMID, NULL);
            check_err(ctl, ctl_err);
            ctl = shmctl(shm_id_mas, IPC_RMID, NULL);
            check_err(ctl, ctl_err);
            ctl = shmctl(shm_id_num, IPC_RMID, NULL);
            check_err(ctl, ctl_err);
            ctl = shmctl(shm_id_x, IPC_RMID, NULL);
            check_err(ctl, ctl_err);
            return 0;
          }
        }

        rcv = msgrcv(id_note, &buf1, sizeof(struct mes), note, MSG_NOERROR);
        if ((rcv < 0) && ((errno == EIDRM) || (errno == EINVAL))) {
          return 0;
        }
        rcv = msgrcv(id_sem, &buf2, 0, HERO, MSG_NOERROR);
      }
    }
  }

  for (i = 0; i < HERO; i++) {
    wait(&status);
  }

  return 0;
}
