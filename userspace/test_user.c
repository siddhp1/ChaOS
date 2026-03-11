// User space syscall wrappers

#define SYS_WRITE 1
#define SYS_EXIT 2

long syscall(long num, long arg0, long arg1, long arg2) {
  register long x8 asm("x8") = num;
  register long x0 asm("x0") = arg0;
  register long x1 asm("x1") = arg1;
  register long x2 asm("x2") = arg2;

  asm volatile("svc #0" : "+r"(x0) : "r"(x8), "r"(x1), "r"(x2) : "memory");

  return x0;
}

long write(int fd, const char* buf, unsigned long count) {
  return syscall(SYS_WRITE, fd, (long)buf, count);
}

void exit(int status) {
  syscall(SYS_EXIT, status, 0, 0);
  while (1);  // Should never reach
}

void _start(void) {
  const char* msg = "Hello from user space!\n";

  // Count string length
  unsigned long len = 0;
  while (msg[len]) {
    len++;
  }

  // Write to stdout
  write(1, msg, len);

  // Loop a bit
  for (volatile int i = 0; i < 5; i++) {
    const char* loop_msg = "User loop iteration\n";
    unsigned long loop_len = 0;
    while (loop_msg[loop_len]) {
      loop_len++;
    }
    write(1, loop_msg, loop_len);

    // Busy wait
    for (volatile int j = 0; j < 10000000; j++);
  }

  // Exit
  exit(42);
}
