TOOLCHAIN ?= aarch64-elf
include toolchains/$(TOOLCHAIN).mk

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -Wall -Wextra -Iinclude -MMD -MP
LDFLAGS = -T linker.ld

# List source files here
SRC = \
	arch/arm64/boot/entry.S \
	arch/arm64/kernel/cpu.c \
	arch/arm64/kernel/exception.c \
	arch/arm64/kernel/vectors.S \
	arch/arm64/mm/fault.c \
	arch/arm64/mm/mmu.c \
	drivers/uart/uart.c \
	kernel/main.c \
	kernel/panic.c \
	kernel/printk.c \
	memory/heap.c \
	memory/kmap.c \
	memory/memory.c \
	memory/page.c \
	memory/physical.c

OBJ = $(SRC:.c=.o)
OBJ := $(OBJ:.S=.o)
DEPS = $(OBJ:.o=.d)

-include $(DEPS)

all: kernel.elf

kernel.elf: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(DEPS) kernel.elf
