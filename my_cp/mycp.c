#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF 4096
#define ENTER 10
#define NBUF 1

struct options {
  int v, f, i, def;
};

struct direct {
  char* fl;
  char* dir;
};

void check_wr_descr(int wr) {
  char wr_error[20] = "Write error";

  if (wr < 0) {
    perror(wr_error);
    errno;
  }
}

void check_cl_descr(int cl) {
  char cl_error[20] = "Close error";

  if (cl < 0) {
    perror(cl_error);
    errno;
  }
}

void verbose(struct direct input, char* pathname) {
  printf("'%s' -> '%s'\n", input.fl, pathname);
}

int force(struct direct input) {
  int fd_wr;

  remove(input.dir);
  fd_wr = open(input.dir, O_CREAT | O_WRONLY | O_TRUNC, 00777);

  return fd_wr;
}

int mywritef(char* buf, int fd, int n);

int interactive(char* pathname) {
  int fd_wr, cl, fd = 1, rd;
  char buf1[20] = "mycp: overwrite '\0";
  char buf2[5] = "'? \0";
  char buf, bin = 'b';

  fd_wr = open(pathname, O_CREAT | O_EXCL, 00777);
  if (fd_wr < 0) {
    mywritef(buf1, fd, 20);
    mywritef(pathname, fd, 1024);
    mywritef(buf2, fd, 5);
    rd = read(0, &buf, NBUF);

    while ((bin != '\0') && (bin != ENTER)) {
      read(0, &bin, 1);
    }
    if (rd < 0)
      return 0;

    if (buf == 'y') {
      return 1;
    } else {
      return 0;
    }
  } else {
    cl = close(fd_wr);
    check_cl_descr(cl);
    return 1;
  }
}

void create_path(struct direct input, char* pathname) {
  int n_dir, n;
  char* file = NULL;
  char slash[2] = "/\0";

  n_dir = strlen(input.dir) - 1;
  n = n_dir + 1;
  strncat(pathname, input.dir, n);
  file = basename(input.fl);
  n = strlen(file);

  if (input.dir[n_dir] == '/') {
    strncat(pathname, file, n);
  } else {
    strncat(pathname, slash, 1);
    strncat(pathname, file, n);
  }
}

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

int mywritef(char* buf, int fd, int n) {
  int x = NBUF, i = 0;

  while ((x > 0) && (i != n)) {
    x = write(fd, (buf + i), NBUF);
    i++;
  }

  if (x < 0)
    return -1;

  return 0;
};

void my_cp_f(char* buf, struct direct input, struct options option) {
  int inter = 0, fd = 0, fd_wr = 0, x = 0, cl = 0, wr = 0;
  struct stat st;
  char error[10] = "mycp";
  char pathname[1024] = "\0";

  fd = open(input.fl, O_RDONLY, 00777);

  if (fd < 0) {
    perror(input.fl);
    errno;
  } else {
    stat(input.dir, &st);

    if (S_ISDIR(st.st_mode)) {
      create_path(input, pathname);

      if (option.v > 0)
        verbose(input, pathname);

      if (option.i > 0)
        inter = interactive(pathname);

      if ((inter == 1) || (option.i == 0))
        fd_wr = open(pathname, O_CREAT | O_WRONLY | O_TRUNC, 00777);
      else
        goto inter0;
    } else {
      if (option.v > 0)
        verbose(input, input.dir);

      if (option.i > 0)
        inter = interactive(input.dir);

      if ((inter == 1) || (option.i == 0)) {
        fd_wr = open(input.dir, O_CREAT | O_WRONLY | O_TRUNC, 00777);
        if ((fd_wr < 0) && (option.f > 0))
          fd_wr = force(input);
      } else
        goto inter0;
    }

    if (fd_wr < 0) {
      perror(input.dir);
      errno;
    } else {
      x = BUF - 1;
      while ((x == BUF - 1) && (wr >= 0)) {
        x = myreadf(buf, fd);

        if (x < 0) {
          perror(error);
          errno;
        } else {
          wr = mywritef(buf, fd_wr, x);
          check_wr_descr(wr);
        }
      }
      cl = close(fd_wr);
      check_cl_descr(cl);
    }
  inter0:
    cl = close(fd);
    check_cl_descr(cl);
  }
};

void my_cp_dir(char* buf, struct direct input, struct options option) {
  int inter, fd = 0, fd_wr = 0, x = 0, cl = 0, wr = 0;
  char error[10] = "mycp";
  char pathname[1024] = "\0";

  fd = open(input.fl, O_RDONLY, 00777);

  if (fd < 0) {
    perror(input.fl);
    errno;
  } else {
    create_path(input, pathname);

    if (option.v > 0)
      verbose(input, pathname);

    if (option.i > 0)
      inter = interactive(pathname);

    if ((inter == 1) || (option.i == 0))
      fd_wr = open(pathname, O_CREAT | O_WRONLY | O_TRUNC, 00777);
    else {
      goto inter00;
    }
    if (fd_wr < 0) {
      perror(input.dir);
      errno;
    } else {
      x = BUF - 1;
      while ((x == BUF - 1) && (wr >= 0)) {
        x = myreadf(buf, fd);

        if (x < 0) {
          perror(error);
          errno;
        } else {
          wr = mywritef(buf, fd_wr, x);
          check_wr_descr(wr);
        }
      }

      cl = close(fd_wr);
      check_cl_descr(cl);
    }
  inter00:
    cl = close(fd);
    check_cl_descr(cl);
  }
};

int main(int argn, char* argv[]) {
  struct options option = {0, 0, 0, 0};
  struct direct input;
  char buf[BUF];
  const char* optstring = ":vfi";
  static struct option long_op[] = {
      {"verbose", 0, 0, 'v'},
      {"force", 0, 0, 'f'},
      {"interactive", 0, 0, 'i'},
      {0, 0, 0, 0},
  };
  struct stat st;
  int option_index = 0;
  int op, i, sum;

  switch (argn) {
    case 1:
      printf("mycp: missing file operand\n");
      break;
    case 2:
      printf("cp: missing destination file operand after '%s'\n", argv[1]);
      break;
    default:

      while ((op = getopt_long(argn, argv, optstring, long_op,
                               &option_index)) != -1) {
        switch (op) {
          case 'v':
            option.v++;
            break;
          case 'f':
            option.f++;
            break;
          case 'i':
            option.i++;
            break;
          case '?':
            option.def++;
            break;
        }
      }
  }

  if (option.def == 0) {
    sum = option.v + option.f + option.i + option.def;
    switch (argn - sum - 1) {
      case 0:
        printf("mycp: missing file operand\n");
        break;
      case 1:
        printf("cp: missing destination file operand after '%s'\n", argv[1]);
        break;
      case 2:
        input.dir = argv[argn - 1];
        input.fl = argv[argn - 2];
        my_cp_f(buf, input, option);
        break;
      default:
        input.dir = argv[argn - 1];
        stat(input.dir, &st);

        if (S_ISDIR(st.st_mode)) {
          for (i = sum + 1; i < argn - 1; i++) {
            input.fl = argv[i];
            my_cp_dir(buf, input, option);
          }
        } else {
          printf("mycp: target '%s' is not a directory\n", input.dir);
        }
        break;
    }
  } else {
    printf("mycp: invalid option -- '%s'\n", argv[1]);
  }

  return 0;
}
