BINARY = blink
LIBOPENCM3 = $(shell pwd)/lib/libopencm3

OBJS = src/main.o

DEVICE = stm32f411ce

OPENCM3_DIR = $(LIBOPENCM3)

LDFLAGS += -nostartfiles

include $(LIBOPENCM3)/mk/genlink-config.mk
include $(LIBOPENCM3)/mk/gcc-config.mk
include $(LIBOPENCM3)/mk/genlink-rules.mk
include $(LIBOPENCM3)/mk/gcc-rules.mk