TOOLCHAIN ?= aarch64-elf
include toolchains/$(TOOLCHAIN).mk

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -Wall -Wextra -Iinclude -MMD -MP -mgeneral-regs-only -Iarch/arm64/include -Iarch/arm64 -Ikernel
DEBUG ?= true
ifeq ($(DEBUG),true)
CFLAGS += -g -O0 -fno-omit-frame-pointer
endif

LDFLAGS = -T linker.ld

# TODO: Switch to globbing here
# List source files here
SRC = \
	arch/arm64/boot/entry.S \
	arch/arm64/kernel/context_switch.S \
	arch/arm64/kernel/cpu.c \
	arch/arm64/kernel/exception.c \
	arch/arm64/kernel/fault.c \
	arch/arm64/kernel/gic.c \
	arch/arm64/kernel/irq.c \
	arch/arm64/kernel/timer.c \
	arch/arm64/kernel/vectors.S \
	arch/arm64/mm/fault.c \
	arch/arm64/mm/mmu.c \
	drivers/uart/uart.c \
	kernel/exec.c \
	kernel/fork.c \
	kernel/initramfs.c \
	kernel/kthread.c \
	kernel/main.c \
	kernel/panic.c \
	kernel/pid.c \
	kernel/printk.c \
	kernel/scheduler.c \
	kernel/sleep.c \
	kernel/string.c \
	kernel/syscall.c \
	kernel/sys_exit.c \
	kernel/sys_getpid.c \
	kernel/sys_read.c \
	kernel/sys_write.c \
	kernel/task.c \
	kernel/uaccess.c \
	kernel/uthread.c \
	kernel/vm.c \
	kernel/wait.c \
	mm/heap.c \
	mm/kmap.c \
	mm/memory.c \
	mm/page.c \
	mm/phys.c

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

