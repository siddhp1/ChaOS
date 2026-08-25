CROSS_COMPILE ?= aarch64-linux-gnu-

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
AS      := $(CROSS_COMPILE)as
AR      := $(CROSS_COMPILE)ar
NM      := $(CROSS_COMPILE)nm
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
READELF := $(CROSS_COMPILE)readelf

DEBUG ?= true

COMMON_CFLAGS := -ffreestanding -nostdlib -nostartfiles -Wall -Wextra -MMD -MP -mgeneral-regs-only

ifeq ($(DEBUG),true)
COMMON_CFLAGS += -g -O0 -fno-omit-frame-pointer
endif
