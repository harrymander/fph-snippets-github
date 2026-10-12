# Must define PRINTF_DIR to point to directory containing this file

ifndef PRINTF_DIR
$(error PRINTF_DIR undefined)
endif

VPATH += $(PRINTF_DIR)
SRC += printf.c
INCLUDES += -I$(PRINTF_DIR)/include
CFLAGS += \
	-DPRINTF_SUPPORT_DECIMAL_SPECIFIERS=0 \
	-DPRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS=0
