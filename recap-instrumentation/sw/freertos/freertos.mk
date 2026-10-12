# Must define FREERTOS_DIR to point to the directory that contains this file
# (not the kernel directory). May also redefine FREERTOS_PORT and optionally
# provide FREERTOS_HEAP_IMPL (leave undefined to use static allocation).

ifndef FREERTOS_DIR
$(error FREERTOS_DIR undefined)
endif

# Note Cortex-M4s without an FPU should use the Cortex-M3 port
FREERTOS_PORT ?= GCC/ARM_CM3

FREERTOS_SRC_DIR = $(FREERTOS_DIR)/FreeRTOS-Kernel
FREERTOS_PORT_DIR = $(FREERTOS_SRC_DIR)/portable/$(FREERTOS_PORT)

FREERTOS_SRC = \
	$(FREERTOS_SRC_DIR)/event_groups.c \
	$(FREERTOS_SRC_DIR)/list.c \
	$(FREERTOS_SRC_DIR)/queue.c \
	$(FREERTOS_SRC_DIR)/tasks.c \
	$(wildcard $(FREERTOS_PORT_DIR)/*.c)

ifneq ($(FREERTOS_HEAP_IMPL),)
FREERTOS_SRC += $(FREERTOS_SRC_DIR)/portable/MemMang/$(FREERTOS_HEAP_IMPL).c
endif

FREERTOS_INCLUDES = -I"$(FREERTOS_SRC_DIR)/include" -I"$(FREERTOS_PORT_DIR)"

SRC += $(notdir $(FREERTOS_SRC)) freertos-port.c
VPATH += $(sort $(dir $(FREERTOS_SRC))) $(FREERTOS_DIR)
INCLUDES += $(FREERTOS_INCLUDES) -I$(FREERTOS_DIR)
