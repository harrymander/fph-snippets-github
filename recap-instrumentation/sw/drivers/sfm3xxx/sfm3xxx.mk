LOCAL_DRIVERS += twi-async

VPATH += $(LOCAL_DRIVERS_DIR)/sfm3xxx
INCLUDES += -I$(LOCAL_DRIVERS_DIR)/sfm3xxx

SRC += sfm3xxx.c
