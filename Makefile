TOOLCHAIN ?= aarch64-elf
include toolchains/$(TOOLCHAIN).mk

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -Wall -Wextra -Iinclude -MMD -MP
LDFLAGS = -T linker.ld

# List source files here
SRC = \
  kernel/main.c \
  kernel/panic.c \
  kernel/printk.c

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
