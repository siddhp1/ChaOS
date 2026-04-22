TOOLCHAIN ?= aarch64-elf
include toolchains/$(TOOLCHAIN).mk

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -Wall -Wextra -Iinclude -MMD -MP -mgeneral-regs-only -Iarch/arm64/include
DEBUG ?= true
ifeq ($(DEBUG),true)
CFLAGS += -g -O0 -fno-omit-frame-pointer
endif

LDFLAGS = -T kernel_linker.ld

SRC = \
	arch/arm64/boot/kernel_boot.S \
	arch/arm64/kernel/cpu.c \
	arch/arm64/kernel/enter_user_mode.S \
	arch/arm64/kernel/exception.c \
	arch/arm64/kernel/gic.c \
	arch/arm64/kernel/irq.c \
	arch/arm64/kernel/timer.c \
	arch/arm64/kernel/vectors.S \
	arch/arm64/mm/fault.c \
	arch/arm64/mm/mmu.c \
	arch/arm64/mm/tlb.c \
	drivers/uart/uart.c \
	kernel/initramfs.c \
	kernel/kthread.c \
	kernel/main.c \
	kernel/panic.c \
	kernel/pid.c \
	kernel/printk.c \
	kernel/scheduler/reaper.c \
	kernel/scheduler/scheduler.c \
	kernel/scheduler/sleep.c \
	kernel/string.c \
	kernel/syscall/syscall.c \
	kernel/syscall/sys_execve.c \
	kernel/syscall/sys_exit.c \
	kernel/syscall/sys_fork.c \
	kernel/syscall/sys_read.c \
	kernel/syscall/sys_write.c \
	kernel/task.c \
	kernel/user_access.c \
	kernel/user_thread.c \
	kernel/scheduler/wait.c \
	mm/heap.c \
	mm/kmap.c \
	mm/kvmalloc.c \
	mm/memory.c \
	mm/page.c \
	mm/pgtable.c \
	mm/user_pgtable.c

OBJ = $(SRC:.c=.o)
OBJ := $(OBJ:.S=.o)
DEPS = $(OBJ:.o=.d)

-include $(DEPS)

INITRAMFS_IMG  = initramfs.img
INITRAMFS_OBJ  = initramfs_blob.o

all: kernel.elf

userspace/init.bin userspace/hello.bin: userspace_build

.PHONY: userspace_build
userspace_build:
	$(MAKE) -C userspace TOOLCHAIN=$(TOOLCHAIN)

$(INITRAMFS_IMG): userspace/init.bin userspace/hello.bin
	python3 tools/mkinitramfs.py -o $@ \
		bin/init=userspace/init.bin \
		bin/hello=userspace/hello.bin

# Convert initramfs binary blob into an object file that can be linked into the kernel
$(INITRAMFS_OBJ): $(INITRAMFS_IMG)
	$(OBJCOPY) -I binary -O elf64-littleaarch64 \
		--rename-section .data=.initramfs,alloc,load,readonly,data,contents \
		$< $@

kernel.elf: $(OBJ) $(INITRAMFS_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(DEPS) kernel.elf $(INITRAMFS_IMG) $(INITRAMFS_OBJ)
	$(MAKE) -C userspace clean
