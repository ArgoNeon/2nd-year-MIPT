#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define _GNU_SOURCE
#include <string.h>

#define BUF 4096
#define BUFCOM 128

char* strcasestr(const char* haystack, const char* needle);

void check_err(int x, const char* name) {
  if (x < 0) {
    perror(name);
    errno;
  }
}

int tokens(char* buf, const char* delim, char** buf_com) {
  char* tok;
  int i = 1;

  tok = strtok(buf, delim);
  buf_com[0] = tok;
  while (tok != NULL) {
    tok = strtok(NULL, delim);
    buf_com[i] = tok;
    i++;
  }

  return i - 1;
}

void print_tok(char** tokens, int n) {
  int i;

  for (i = 0; i < n; i++)
    printf("%d. %s\n", i + 1, tokens[i]);
}

int main(int argn, char* argv[]) {
  const char myshell[20] = "myshell";
  const char delim1[5] = "|\n";
  const char delim2[5] = " ";
  const char exit[20] = "exit";
  const char pipe_err[20] = "Pipe error";
  const char fork_err[20] = "Fork error";
  const char close_err[20] = "Close error";
  int fd[BUFCOM][2];
  char buf[BUF] = "\0";
  char* buf_com[BUFCOM];
  char* com[BUFCOM];
  char *rd_line, *rd_exit;
  int n, i, x, n_com, ppipe, cl;
  pid_t pid = 1;

  while (feof(stdin) == 0) {
    printf("%s: ", myshell);

    rd_line = fgets(buf, BUF, stdin);
    rd_exit = strcasestr(buf, exit);

    if ((rd_line == NULL) || (rd_exit != NULL))
      break;

    n = tokens(buf, delim1, buf_com);
    ppipe = -1;

    for (i = 0; i < n; i++) {
      x = pipe(fd[i]);
      if (x < 0) {
        perror(pipe_err);
        errno;
        break;
      }

      pid = fork();

      if (pid < 0) {
        perror(fork_err);
        errno;
        break;
      }

      if (pid == 0) {
        if (i > 0) {
          dup2(ppipe, 0);
        }
        if (i < (n - 1)) {
          dup2(fd[i][1], 1);
        }

        cl = close(fd[i][0]);
        check_err(cl, close_err);
        cl = close(fd[i][1]);
        check_err(cl, close_err);

        n_com = tokens(buf_com[i], delim2, com);
        execvp(com[0], com);

      } else {
        cl = close(fd[i][1]);
        check_err(cl, close_err);
        if (i > 0) {
          cl = close(ppipe);
          check_err(cl, close_err);
        }
        ppipe = fd[i][0];
      }
    }
    waitpid(pid, NULL, WUNTRACED);
  }

  return 0;
}
