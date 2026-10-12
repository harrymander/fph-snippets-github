
LOCAL_DRIVERS += tmr

VPATH += $(LOCAL_DRIVERS_DIR)/status
INCLUDES += -I$(LOCAL_DRIVERS_DIR)/status

SRC += status.c
