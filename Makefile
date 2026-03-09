TOOLCHAIN ?= aarch64-elf
include toolchains/$(TOOLCHAIN).mk

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -Wall -Wextra -Iinclude -MMD -MP -mgeneral-regs-only -Iarch/arm64/include
DEBUG ?= true
ifeq ($(DEBUG),true)
CFLAGS += -g -O0 -fno-omit-frame-pointer
endif

LDFLAGS = -T linker.ld

SRC = \
	arch/arm64/boot/entry.S \
	arch/arm64/kernel/context_switch.S \
	arch/arm64/kernel/cpu.c \
	arch/arm64/kernel/exception.c \
	arch/arm64/kernel/gic.c \
	arch/arm64/kernel/irq.c \
	arch/arm64/kernel/timer.c \
	arch/arm64/kernel/vectors.S \
	arch/arm64/mm/fault.c \
	arch/arm64/mm/mmu.c \
	arch/arm64/mm/pgtable.c \
	arch/arm64/mm/user_pgtable.c \
	drivers/uart/uart.c \
	kernel/kthread.c \
	kernel/main.c \
	kernel/panic.c \
	kernel/pid.c \
	kernel/printk.c \
	kernel/scheduler.c \
	kernel/sleep.c \
	kernel/string.c \
	kernel/task.c \
	kernel/user_thread.c \
	kernel/wait.c \
	mm/heap.c \
	mm/kmap.c \
	mm/kvmalloc.c \
	mm/memory.c \
	mm/page.c \
	mm/phys.c

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
