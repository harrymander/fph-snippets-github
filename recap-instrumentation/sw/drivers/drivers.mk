
include $(foreach driver, $(LOCAL_DRIVERS), $(LOCAL_DRIVERS_DIR)/$(driver)/$(driver).mk)

# Perform second pass for the peripherals that depend on other peripherals
include $(foreach driver, $(LOCAL_DRIVERS), $(LOCAL_DRIVERS_DIR)/$(driver)/$(driver).mk)

