TOOLCHAIN ?= aarch64-elf
include ../toolchains/$(TOOLCHAIN).mk

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -Wall -Wextra -I. -MMD -MP -mgeneral-regs-only
DEBUG ?= true
ifeq ($(DEBUG),true)
CFLAGS += -g -O0 -fno-omit-frame-pointer
endif

LDFLAGS = -T linker.ld

LIBC_SRC = \
	libc/syscall.c \
	libc/write.c \
	libc/exit.c \
	libc/fork.c \
	libc/read.c \
	libc/write.c

SRC = \
	userspace_entry.S \
	init.c \
	$(LIBC_SRC)

OBJ = $(SRC:.c=.o)
OBJ := $(OBJ:.S=.o)
DEPS = $(OBJ:.o=.d)

-include $(DEPS)

LIBC_OBJ  = $(patsubst %.c,%.o,$(LIBC_SRC))
ENTRY_OBJ = userspace_entry.o

PROGRAMS = init hello
BINS     = $(patsubst %,%.bin,$(PROGRAMS))
ELFS     = $(patsubst %,%.elf,$(PROGRAMS))

all: $(BINS)

.SECONDARY: $(ELFS) $(OBJ)

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.elf: %.o $(ENTRY_OBJ) $(LIBC_OBJ)
	$(LD) $(LDFLAGS) -o $@ $(ENTRY_OBJ) $< $(LIBC_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(DEPS) $(BINS) $(ELFS)
