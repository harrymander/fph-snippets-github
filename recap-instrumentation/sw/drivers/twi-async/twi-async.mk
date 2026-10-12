PERIPHERALS += twi

VPATH += $(LOCAL_DRIVERS_DIR)/twi-async
INCLUDES += -I$(LOCAL_DRIVERS_DIR)/twi-async

SRC += twi-async.c
