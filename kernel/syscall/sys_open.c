#include "fs/console.h"
#include "fs/file.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/string.h"
#include "kernel/user_access.h"
#include "syscall_handlers.h"

long sys_open(long user_path_addr, long flags, long mode, long a3, long a4,
              long a5) {
  (void)a3;
  (void)a4;
  (void)a5;

  if (user_path_addr == 0) return -1;

  char path[128];
  if (copy_user_cstr(path, (const char*)user_path_addr, sizeof(path)) < 0) {
    return -1;
  }

  struct file* file = NULL;

  // Temporary hardcoding until VFS
  if (strcmp(path, "/dev/console") == 0) {
    file = console_file_open((int)flags);
  }

  if (!file) return -1;

  int fd = fd_install(current_task, file);
  if (fd < 0) {
    file_unref(file);
    return fd;
  }

  return fd;
}
