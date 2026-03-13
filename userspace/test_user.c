#define SYS_WRITE 0
#define SYS_EXIT 1
#define SYS_FORK 4

long syscall(long num, long a0, long a1, long a2, long a3, long a4, long a5);

long write(long fd, const void* buf, long count);

void exit(long status);

long fork(void);

void _start(void) {
  const char* msg1 = "Before fork\n";
  write(1, msg1, 12);

  long pid = fork();

  if (pid == 0) {
    const char* msg = "I am the child!\n";
    write(1, msg, 16);

    for (volatile int i = 0; i < 1000000; i++);

    const char* msg2 = "Child exiting\n";
    write(1, msg2, 14);

    exit(0);
  } else if (pid > 0) {
    const char* msg = "I am the parent, child PID=";
    write(1, msg, 28);

    char digit = '0' + (char)(pid % 10);
    write(1, &digit, 1);
    write(1, "\n", 1);

    for (volatile int i = 0; i < 2000000; i++);

    const char* msg2 = "Parent exiting\n";
    write(1, msg2, 15);

    exit(0);
  } else {
    const char* msg = "Fork failed!\n";
    write(1, msg, 13);

    exit(1);
  }
}

long syscall(long num, long a0, long a1, long a2, long a3, long a4, long a5) {
  register long x8 __asm__("x8") = num;
  register long x0 __asm__("x0") = a0;
  register long x1 __asm__("x1") = a1;
  register long x2 __asm__("x2") = a2;
  register long x3 __asm__("x3") = a3;
  register long x4 __asm__("x4") = a4;
  register long x5 __asm__("x5") = a5;

  __asm__ volatile("svc #0"
                   : "+r"(x0)
                   : "r"(x8), "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5)
                   : "memory");

  return x0;
}

long write(long fd, const void* buf, long count) {
  return syscall(SYS_WRITE, fd, (long)buf, count, 0, 0, 0);
}

void exit(long status) {
  syscall(SYS_EXIT, status, 0, 0, 0, 0, 0);
  while (1);
}

long fork(void) { return syscall(SYS_FORK, 0, 0, 0, 0, 0, 0); }
