CROSS   = aarch64-elf-
CC      = $(CROSS)gcc
LD      = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

CFLAGS  = -ffreestanding -nostdlib -nostartfiles -Wall -Wextra
LDFLAGS = -T linker.ld

SRC = \
  arch/arm64/boot.S \
  kernel/main.c

OBJ = $(SRC:.c=.o)
OBJ := $(OBJ:.S=.o)

all: kernel.elf

kernel.elf: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) kernel.elf
