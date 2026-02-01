#ifndef PRINTK_H
#define PRINTK_H

#include <stdint.h>

void printk_hex_u32(uint32_t num);
void printk_hex_u64(uint64_t num);
void printk(const char* str);

#endif
