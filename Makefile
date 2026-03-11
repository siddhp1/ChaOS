TOOLCHAIN ?= aarch64-elf
include toolchains/$(TOOLCHAIN).mk

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -Wall -Wextra -Iinclude -MMD -MP -mgeneral-regs-only -Iarch/arm64/include
DEBUG ?= true
ifeq ($(DEBUG),true)
CFLAGS += -g -O0 -fno-omit-frame-pointer
endif

LDFLAGS = -T linker.ld

# TODO: Switch to globbing
SRC = \
	arch/arm64/boot/entry.S \
	arch/arm64/kernel/context_switch.S \
	arch/arm64/kernel/cpu.c \
	arch/arm64/kernel/enter_usermode.S \
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
	kernel/syscall.c \
	kernel/sys_exit.c \
	kernel/sys_write.c \
	kernel/task.c \
	kernel/user_access.c \
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

USER_SRC = userspace/test_user.c
USER_OBJ = $(USER_SRC:.c=.o)
USER_ELF = userspace/test_user.elf
USER_BIN = userspace/test_user.bin
USER_OBJ_EMBED = userspace/test_user_bin.o

-include $(DEPS)

all: kernel.elf $(USER_BIN)

kernel.elf: $(OBJ) $(USER_OBJ_EMBED)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

userspace/%.o: userspace/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(USER_ELF): $(USER_OBJ)
	$(LD) -T userspace/linker.ld $(USER_OBJ) -o $@

$(USER_BIN): $(USER_ELF)
	$(OBJCOPY) -O binary $< $@

# Convert user binary to object file for embedding in kernel
$(USER_OBJ_EMBED): $(USER_BIN)
	$(OBJCOPY) -I binary -O elf64-littleaarch64 -B aarch64 \
		--rename-section .data=.rodata,alloc,load,readonly,data,contents \
		$< $@

clean:
	rm -f $(OBJ) $(DEPS) kernel.elf kernel.bin
	rm -f $(USER_OBJ) $(USER_ELF) $(USER_BIN) $(USER_OBJ_EMBED)
