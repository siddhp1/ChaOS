#include "kernel/initramfs.h"

#include <stddef.h>

#include "kernel/printk.h"
#include "kernel/string.h"

#define MAX_FILES 32

extern char initramfs_start[];
extern char initramfs_end[];

static struct initramfs_file files[MAX_FILES];
static int file_count = 0;

void initramfs_init(void) {
  char* init_ptr = initramfs_start;

  while (init_ptr < initramfs_end && file_count < MAX_FILES) {
    files[file_count].name = init_ptr;
    init_ptr += strlen(init_ptr) + 1;

    size_t size = *(size_t*)init_ptr;
    files[file_count].size = size;
    init_ptr += sizeof(size_t);

    files[file_count].data = init_ptr;
    init_ptr += size;

    printk("initramfs: file ");
    printk(files[file_count].name);
    printk("\n");

    file_count++;
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
