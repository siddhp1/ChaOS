#include "kernel/initramfs.h"

#include <stdbool.h>
#include <stddef.h>

#include "kernel/align.h"
#include "kernel/panic.h"
#include "kernel/printk.h"
#include "kernel/string.h"
#include "kernel/user_thread.h"

#define INIT_FILE_PATH "bin/init"

#define CPIO_NEWC_HDR_SIZE 110
#define CPIO_NEWC_MAGIC "070701"
#define CPIO_MODE_OFFSET 14
#define CPIO_FILESIZE_OFFSET 54
#define CPIO_NAMESIZE_OFFSET 94
#define CPIO_TRAILER "TRAILER!!!"

#define MAX_FILES 32

extern char initramfs_start[];
extern char initramfs_end[];

static struct initramfs_file files[MAX_FILES];
static int file_count = 0;

struct task* task_init = NULL;

// TODO: Make a general library function to convert strings to integers
static inline int hex_to_dec(unsigned char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

static bool parse_hex(const char* s, size_t n, unsigned long* out) {
  unsigned long v = 0;
  for (size_t i = 0; i < n; i++) {
    int d = hex_to_dec((unsigned char)s[i]);
    if (d < 0) return false;
    v = (v << 4) | (unsigned long)d;
  }
  *out = v;
  return true;
}

void initramfs_init(void) {
  char* curr_hdr = initramfs_start;
  file_count = 0;

  while (curr_hdr + CPIO_NEWC_HDR_SIZE <= initramfs_end &&
         file_count < MAX_FILES) {
    if (strncmp(curr_hdr, CPIO_NEWC_MAGIC, 6) != 0) break;  // Not a newc header

    unsigned long mode = 0;
    unsigned long filesize = 0;
    unsigned long namesize = 0;

    if (!parse_hex(curr_hdr + CPIO_MODE_OFFSET, 8, &mode) ||
        !parse_hex(curr_hdr + CPIO_FILESIZE_OFFSET, 8, &filesize) ||
        !parse_hex(curr_hdr + CPIO_NAMESIZE_OFFSET, 8, &namesize))
      break;

    // Check name is within bounds and name is not trailer
    char* name = curr_hdr + CPIO_NEWC_HDR_SIZE;
    if (namesize == 0 || name + namesize > initramfs_end) break;
    if (strcmp(name, CPIO_TRAILER) == 0) break;

    // Check data is within bounds
    char* data = align_up(name + namesize, 4);
    if (data + filesize > initramfs_end) break;

    // Only load regular files
    if ((mode & 0170000) == 0100000) {
      files[file_count].name = name;
      files[file_count].data = data;
      files[file_count].size = (size_t)filesize;

      printk("initramfs: file %s\n", files[file_count].name);

      file_count++;
    }

    curr_hdr = align_up(data + filesize, 4);
  }
}

struct initramfs_file* initramfs_lookup(const char* filename) {
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, filename) == 0) {
      return &files[i];
    }
  }
  return NULL;
}

struct task* load_init(void) {
  struct initramfs_file* init_file = initramfs_lookup(INIT_FILE_PATH);
  if (!init_file) {
    printk("initramfs: missing %s\n", INIT_FILE_PATH);
    return NULL;
  }

  printk("initramfs: launching %s size=%lx\n", INIT_FILE_PATH,
         (uint64_t)init_file->size);

  task_init = create_user_process(init_file->data, init_file->size);
  if (!task_init) {
    panic("initramfs: failed to create init process\n");
  }

  return task_init;
}
