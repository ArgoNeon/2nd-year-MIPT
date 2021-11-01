#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argn, char* argv[]) {
  pid_t pid;
  int n, status, i = 0;

  switch (argn) {
    case 2:
      n = atoi(argv[1]);

      while (i < n) {
        pid = fork();
        if (pid == 0) {
          printf("Child%d:\npid = %d\nppid = %d\n", i, getpid(), getppid());
        } else {
          wait(&status);
          if (WIFEXITED(status)) {
            printf("Parent%d:\npid = %d\nppid = %d\n", i, getpid(), getppid());
            break;
          }
        }
        i++;
      }
      return 0;

    default:
      return 0;
  }
}
