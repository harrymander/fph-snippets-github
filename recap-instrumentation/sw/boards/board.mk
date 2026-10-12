BOARD ?= v3

RUN_MODE = ROM

ifndef OPT
OPT = -O0
endif

MMCULIB_DIR = $(TOPDIR)/mmculib
MAT91LIB_DIR = $(TOPDIR)/mat91lib
LOCAL_DRIVERS_DIR = $(TOPDIR)/drivers

ifndef BOARD
$(error BOARD not defined)
endif

MCU = SAM4SA16B

INCLUDES += -I$(TOPDIR)/boards -I$(TOPDIR)/boards/$(BOARD)

CFLAGS += -DRECAP_INSTRUMENTATION_$(shell echo "$(BOARD)" | tr a-z A-Z)

include $(LOCAL_DRIVERS_DIR)/drivers.mk
include $(MMCULIB_DIR)/mmculib.mk
