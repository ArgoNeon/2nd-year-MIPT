#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argn, char* args[]) {
  int i, x;
  char* key = "-n";

  if (argn > 1) {
    x = strcmp(args[1], key);

    if (x == 0) {
      if (argn > 2) {
        for (i = 2; i < argn - 1; i++) {
          printf("%s ", args[i]);
        }
        printf("%s", args[i]);
      }
    } else {
      for (i = 1; i < argn - 1; i++) {
        printf("%s ", args[i]);
      }
      printf("%s", args[i]);

      printf("\n");
    }
  } else {
    printf("\n");
  }

  return 0;
}
