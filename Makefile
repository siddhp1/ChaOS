# Toolchain & configuration
TOOLCHAIN ?= aarch64-elf
include toolchains/$(TOOLCHAIN).mk

DEBUG ?= true
PLATFORM ?= qemu_virt

# Flags
LDFLAGS = -T arch/arm64/platform/$(PLATFORM)/kernel_linker.ld

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -Wall -Wextra -MMD -MP -mgeneral-regs-only

CFLAGS += -Iinclude
CFLAGS += -Iarch/arm64/include
CFLAGS += -Iarch/arm64/platform/$(PLATFORM)/include

ifeq ($(DEBUG),true)
CFLAGS += -g -O0 -fno-omit-frame-pointer
endif

# Sources & derived
SRC = \
	arch/arm64/boot/kernel_boot.S \
	arch/arm64/kernel/cpu.c \
	arch/arm64/kernel/enter_user_mode.S \
	arch/arm64/kernel/exception.c \
	arch/arm64/kernel/irq.c \
	arch/arm64/kernel/vectors.S \
	arch/arm64/mm/fault.c \
	arch/arm64/mm/mmu.c \
	arch/arm64/mm/tlb.c \
	arch/arm64/platform/$(PLATFORM)/drivers/irq_controller.c \
	arch/arm64/platform/$(PLATFORM)/drivers/timer.c \
	arch/arm64/platform/$(PLATFORM)/drivers/uart.c \
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
	kernel/syscall/sys_wait.c \
	kernel/syscall/sys_waitpid.c \
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

INITRAMFS_ROOT := initramfs_root

# Top-level targets
.DEFAULT_GOAL := all
.PHONY: all image clean userspace_build

all: kernel.elf
image: kernel8.img

# Userspace & initramfs
userspace/init.bin userspace/hello.bin userspace/sh.bin : userspace_build
userspace_build:
	$(MAKE) -C userspace TOOLCHAIN=$(TOOLCHAIN)

# Archive initramfs as cpio newc image (binary blob)
$(INITRAMFS_IMG): userspace/init.bin userspace/hello.bin userspace/sh.bin
	rm -rf $(INITRAMFS_ROOT)
	mkdir -p $(INITRAMFS_ROOT)/bin
	cp userspace/init.bin $(INITRAMFS_ROOT)/bin/init
	cp userspace/hello.bin $(INITRAMFS_ROOT)/bin/hello
	cp userspace/sh.bin $(INITRAMFS_ROOT)/bin/sh
	( cd $(INITRAMFS_ROOT) && \
		find . -mindepth 1 -printf '%P\n' | \
		LC_ALL=C sort | \
		cpio -o -H newc --quiet \
	) > $@

# Convert initramfs binary blob into an object file that can be linked into the kernel
$(INITRAMFS_OBJ): $(INITRAMFS_IMG)
	$(OBJCOPY) -I binary -O elf64-littleaarch64 \
		--rename-section .data=.initramfs,alloc,load,readonly,data,contents \
		$< $@

# Build rules
kernel.elf: $(OBJ) $(INITRAMFS_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

# Packaging
kernel8.img: kernel.elf
	$(OBJCOPY) -O binary $< $@

# Clean
clean:
	rm -rf $(OBJ) $(DEPS) $(INITRAMFS_IMG) $(INITRAMFS_OBJ) $(INITRAMFS_ROOT) kernel.elf kernel8.img
	$(MAKE) -C userspace clean
