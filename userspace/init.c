#include "libc/libc.h"

int main(void) {
  while (1) {
    int pid = fork();
    if (pid == 0) {
      execve("bin/sh", 0);
      write(1, "exec failed\n", 12);  // execve should not return
      exit(1);
    }

    int status;
    waitpid(pid, &status);
  }
}
