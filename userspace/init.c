#include "libc/libc.h"

static int streq(const char *a, const char *b) {
  while (*a && *b) {
    if (*a != *b) return 0;
    a++;
    b++;
  }
  return *a == *b;
}

int main(void) {
  write(1, "=== OS Shell ===\n", 17);

  char buf[64];

  while (1) {
    write(1, "$ ", 2);

    long n = read(0, buf, 63);
    if (n <= 0) continue;
    buf[n] = '\0';

    while (n > 0 && (buf[n - 1] == ' ' || buf[n - 1] == '\n')) {
      buf[--n] = '\0';
    }
    if (n == 0) continue;

    if (streq(buf, "hello")) {
      int pid = fork();
      if (pid == 0) {
        execve("bin/hello", 0);
        write(1, "exec failed\n", 12);  // execve should not return
      }
      // Parent continues
    } else if (streq(buf, "exit")) {
      write(1, "Goodbye\n", 8);
      exit(0);
    } else {
      write(1, "unknown: ", 9);
      long len = 0;
      while (buf[len]) len++;
      write(1, buf, len);
      write(1, "\n", 1);
    }
  }
}
