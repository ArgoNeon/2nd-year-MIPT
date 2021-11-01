#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF 4096
#define NBUF 1
#define ENTER 10

int myread(char* buf) {
  int x = NBUF, i = 0, en = 0;
  void* vbuf = buf;

  while ((x > 0) && (en != ENTER) && (i < BUF)) {
    x = read(0, (vbuf + i), NBUF);
    en = buf[i];
    i++;
  }

  if ((i != 1) && (x == 0))
    return i - 1;

  if ((x == 0) && (i == 1))
    return 0;

  if (x < 0)
    return -1;

  return i;
};

int mywrite(void* buf, int n) {
  int x = NBUF, i = 0;

  while ((x > 0) && (i != n)) {
    x = write(1, (buf + i), NBUF);
    i++;
  }
  if (x < 0)
    return -1;

  return 0;
};

void nameless_cat(char* buf) {
  int x = NBUF, wr = 0;
  char wrerror[20] = "Write error";
  char rderror[20] = "Read error";

  while ((x > 0) && (wr >= 0)) {
    x = myread(buf);

    if (x < 0) {
      perror(rderror);
      errno;
    }

    wr = mywrite(buf, x);

    if (wr < 0) {
      perror(wrerror);
      errno;
    }
  }
};

int myreadf(char* buf, int fd) {
  int x = NBUF, i = 0;

  while ((x > 0) && (i < BUF)) {
    x = read(fd, (buf + i), NBUF);
    i++;
  }

  if (x < 0)
    return -1;

  return i - 1;
};

int mywritef(char* buf, int n) {
  int x = NBUF, i = 0;

  while ((x > 0) && (i != n)) {
    x = write(1, (buf + i), NBUF);
    i++;
  }

  if (x < 0) {
    return -1;
  }

  return 0;
};

void name_cat(char* buf, const char** args, int argn) {
  int i = 0, fd = 0, x = 0, cl = 0, wr = 0;
  char wrerror[20] = "Write error";
  char rderror[20] = "Read error";
  char clerror[20] = "Close error";

  for (i = 1; i < argn; i++) {
    fd = open(args[i], O_RDONLY, 0777);

    if (fd < 0) {
      perror(args[i]);
      errno;
    } else {
      x = BUF - 1;
      while ((x == BUF - 1) && (wr >= 0)) {
        x = myreadf(buf, fd);

        if (x < 0) {
          perror(rderror);
          errno;
        } else {
          wr = mywritef(buf, x);

          if (wr < 0) {
            perror(wrerror);
            errno;
          }
        }
      }
      cl = close(fd);

      if (cl < 0) {
        perror(clerror);
        errno;
      }
    }
  }
};

int main(int argn, const char** args) {
  char buf[BUF];

  if (argn == 1) {
    nameless_cat(buf);

  } else {
    name_cat(buf, args, argn);
  }
  return 0;
}
