TOOLCHAIN ?= aarch64-elf
include ../toolchains/$(TOOLCHAIN).mk

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -Wall -Wextra -I. -MMD -MP -mgeneral-regs-only
DEBUG ?= true
ifeq ($(DEBUG),true)
CFLAGS += -g -O0 -fno-omit-frame-pointer
endif

LDFLAGS = -T user_linker.ld

LIBC_SRC = \
	libc/execve.c \
	libc/exit.c \
	libc/fork.c \
	libc/read.c \
	libc/syscall.c \
	libc/string.c \
	libc/write.c

SRC = \
	hello.c \
	init.c \
	user_boot.S \
	$(LIBC_SRC)

OBJ = $(SRC:.c=.o)
OBJ := $(OBJ:.S=.o)
DEPS = $(OBJ:.o=.d)

-include $(DEPS)

LIBC_OBJ  = $(patsubst %.c,%.o,$(LIBC_SRC))
ENTRY_OBJ = user_boot.o

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
