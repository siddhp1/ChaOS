#include "kernel/initramfs.h"

#include <stddef.h>

#include "kernel/printk.h"
#include "kernel/string.h"
#include "kernel/user_thread.h"

#define INIT_FILE_PATH "bin/init"

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

    printk("initramfs: file %s\n", files[file_count].name);

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

struct task* load_init(void) {
  struct initramfs_file* init_file = initramfs_lookup(INIT_FILE_PATH);
  if (!init_file) {
    printk("initramfs: missing %s\n", INIT_FILE_PATH);
    return NULL;
  }

  printk("initramfs: launching %s size=%lx\n", INIT_FILE_PATH,
         (uint64_t)init_file->size);

  return create_user_process(init_file->data, init_file->size);
}
