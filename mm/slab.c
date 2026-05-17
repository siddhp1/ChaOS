#include "mm/slab.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/align.h"
#include "kernel/irq.h"
#include "mm/kmap.h"
#include "mm/page.h"

// TODO: Add more object sizes here
// Currently, we only need 32 for initramfs_file and 128 for task
static const uint16_t obj_sizes[] = {32, 128};
#define CACHE_CNT (sizeof(obj_sizes) / sizeof(uint16_t))
#define MAGIC 0x12345678UL

struct slab_obj {
  struct slab_obj* next;
};

struct slab_page {
  uint64_t magic;
  struct slab_obj* free_list;
  struct slab_page* next;
  struct slab_cache* cache;
  uint8_t num_in_use;
};

struct slab_cache {
  size_t obj_size;
  struct slab_page* slabs;
};

static struct slab_cache caches[CACHE_CNT];

void slab_init(void) {
  for (size_t i = 0; i < CACHE_CNT; ++i) {
    caches[i].obj_size = obj_sizes[i];
    caches[i].slabs = NULL;  // Slab pages are created on first alloc
  }
}

bool slab_grow(struct slab_cache* cache) {
  struct page* page = alloc_page();
  if (page == NULL) return false;

  struct slab_page* sp = (struct slab_page*)kmap(page);
  sp->magic = (uintptr_t)sp ^ MAGIC;
  sp->num_in_use = 0;

  uint8_t* base = (uint8_t*)sp;
  uint8_t* start = (uint8_t*)align_up(base + sizeof(struct slab_page), 16);
  uint8_t* end = base + PAGE_SIZE;

  sp->free_list = NULL;
  for (uint8_t* ptr = start; ptr + cache->obj_size <= end;
       ptr += cache->obj_size) {
    struct slab_obj* obj = (struct slab_obj*)ptr;
    obj->next = sp->free_list;
    sp->free_list = obj;
  }

  if (sp->free_list == NULL) {
    free_page(page);
    return false;
  }

  sp->cache = cache;

  uint64_t daif = irq_save();
  sp->next = cache->slabs;
  cache->slabs = sp;
  irq_restore(daif);

  return true;
}

void* slab_alloc(size_t size) {
  if (size == 0) return NULL;

  // Determine which cache to allocate from
  struct slab_cache* cache = NULL;
  for (size_t i = 0; i < CACHE_CNT; ++i) {
    if (caches[i].obj_size >= size) {
      cache = &caches[i];
      break;
    }
  }
  if (!cache) return NULL;  // No cache exists with obj_size >= size

  uint64_t daif = irq_save();
  struct slab_obj* so = NULL;
  struct slab_page* sp = cache->slabs;
  while (sp != NULL) {
    if (sp->free_list != NULL) {
      so = sp->free_list;
      sp->free_list = so->next;
      sp->num_in_use++;
      irq_restore(daif);
      return (void*)so;
    }
    sp = sp->next;
  }
  irq_restore(daif);

  // If slab cache is full, grow cache by one page
  if (!slab_grow(cache)) return NULL;

  daif = irq_save();
  sp = cache->slabs;
  if (!sp || !sp->free_list) {
    irq_restore(daif);
    return NULL;
  }
  so = sp->free_list;
  sp->free_list = so->next;
  sp->num_in_use++;
  irq_restore(daif);

  return (void*)so;
}

void slab_free(void* ptr) {
  if (ptr == NULL) return;

  struct slab_page* sp = (struct slab_page*)align_down(ptr, PAGE_SIZE);

  // Validate magic value to ensure memory is only freed on slab-allocated pages
  if (((sp->magic ^ MAGIC) != (uintptr_t)sp)) return;

  bool should_free_page = false;
  struct slab_cache* cache = sp->cache;

  uint64_t daif = irq_save();

  struct slab_obj* obj = (struct slab_obj*)ptr;
  obj->next = sp->free_list;
  sp->free_list = obj;

  if (sp->num_in_use == 0) {
    irq_restore(daif);
    return;
  }

  sp->num_in_use--;

  if (sp->num_in_use == 0) {
    if (cache->slabs == sp) {
      cache->slabs = sp->next;
    } else {
      struct slab_page* prev = cache->slabs;
      while (prev && prev->next != sp) prev = prev->next;
      if (prev) prev->next = sp->next;
    }
    should_free_page = true;
  }

  irq_restore(daif);

  if (should_free_page) {
    struct page* page = phys_to_page(kernel_to_phys((uintptr_t)sp));
    if (page) free_page(page);
  }
}
