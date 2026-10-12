-include .env.mk

TOPDIR = .

BUILD_DIR ?= build

TARGET ?= recap-instrumentation.elf

ifneq ($(if $(GENERATE_COMPILE_COMMANDS),$(GENERATE_COMPILE_COMMANDS),0),0)
all: compile-commands
endif

FREERTOS_DIR = $(TOPDIR)/freertos
FREERTOS_PORT = GCC/ARM_CM3

FATFS_DIR = $(TOPDIR)/fatfs

PRINTF_DIR = $(TOPDIR)/printf

VERSION ?= $(shell ./scripts/version.sh)
ifeq ($(VERSION),)
$(error Failed to determine version)
endif

APPCONFIG_HEADER = $(BUILD_DIR)/appconfig.h

TEST_TIMING ?= 0

ENABLE_H2S ?= 0

USB_CONTROL ?= 1
BTN_CONTROL ?= 1

ADC_VREF ?= 5000

H2S_SENSOR_SERIAL_NUMBER ?= 072116020232
H2S_SENSOR_PART_NUMBER ?= 110303
H2S_SENSOR_GAS_CODE ?= H2S
H2S_SENSOR_DATE_CODE ?= 1608
H2S_SENSOR_SENSITIVITY_CODE ?= 287.59

define generate_appconfig =
mkdir -p $(dir $(APPCONFIG_HEADER)) &&
sed \
	-e "s/{{VERSION_STR}}/$(VERSION)/g" \
	-e "s/{{TEST_TIMING}}/$(TEST_TIMING)/g" \
	-e "s/{{ENABLE_H2S}}/$(ENABLE_H2S)/g" \
	-e "s/{{USB_CONTROL}}/$(USB_CONTROL)/g" \
	-e "s/{{BTN_CONTROL}}/$(BTN_CONTROL)/g" \
	-e "s/{{ADC_VREF}}/$(ADC_VREF)/g" \
	-e "s/{{H2S_SENSOR_SERIAL_NUMBER}}/$(H2S_SENSOR_SERIAL_NUMBER)/g" \
	-e "s/{{H2S_SENSOR_PART_NUMBER}}/$(H2S_SENSOR_PART_NUMBER)/g" \
	-e "s/{{H2S_SENSOR_GAS_CODE}}/$(H2S_SENSOR_GAS_CODE)/g" \
	-e "s/{{H2S_SENSOR_DATE_CODE}}/$(H2S_SENSOR_DATE_CODE)/g" \
	-e "s/{{H2S_SENSOR_SENSITIVITY_CODE}}/$(H2S_SENSOR_SENSITIVITY_CODE)/g" \
  src/appconfig.template.h > $(APPCONFIG_HEADER).tmp &&
if ! [ -f $(APPCONFIG_HEADER) ] || ! diff >/dev/null -q $(APPCONFIG_HEADER).tmp $(APPCONFIG_HEADER); then
  mv $(APPCONFIG_HEADER).tmp $(APPCONFIG_HEADER);
fi
endef

ifneq ($(if $(VERBOSE),$(VERBOSE),0),0)
$(info $(generate_appconfig))
endif
$(shell $(generate_appconfig))

OPT = -O3

SRC += \
	main.c \
	build-id.c \
	control.c \
	error.c \
	hwid.c \
	indicator.c \
	record.c \
	sensors.c \
	usbio.c

CFLAGS += \
	-std=gnu11 \
	-Werror \
	-Werror=vla \
	-ffile-prefix-map=$(shell pwd)=. \
	-fdebug-prefix-map=$(BUILD_DIR)=.
LDFLAGS += -Wl,--build-id=sha1 -Wl,--wrap=_sbrk -Wl,--wrap=_exit -Lscripts -Toverlay.ld
VPATH += src
PERIPHERALS += pwm
LOCAL_DRIVERS += max1167 sdp810 sfm3xxx tmr twi-async
DRIVERS += usb_cdc
INCLUDES += -Isrc -I$(BUILD_DIR)

ifneq ($(ENABLE_H2S),0)
    LOCAL_DRIVERS += h2s
endif

ifneq ($(BTN_CONTROL),0)
    SRC += button.c
endif

OPENOCD_EXTRA_ARGS += -f scripts/rtos-openocd.cfg

include $(PRINTF_DIR)/printf.mk
include $(FREERTOS_DIR)/freertos.mk
include $(TOPDIR)/boards/board.mk

ifeq ($(IGNORE_CC_VERSION),)
EXPECTED_CC_VERSION = 14.2.1
CC_VERSION = $(shell $(CC) -dumpversion)
ifneq ($(CC_VERSION), $(EXPECTED_CC_VERSION))
    $(error Expected $(CC) version $(EXPECTED_CC_VERSION), got $(CC_VERSION))
endif
endif

.PHONY: print-version
print-version:
	@echo $(VERSION)

.PHONY: check
check:
	cppcheck --force --enable=style --std=c99 --error-exitcode=1 --inline-suppr \
	-i tools -i test -i mmculib -i mat91lib -i fatfs \
	-i freertos/FreeRTOS-Kernel -i demos \
	.

.PHONY: print-config
print-config:
	@echo "BUILD_DIR                   = $(BUILD_DIR)"
	@echo "TARGET                      = $(TARGET)"
	@echo "FREERTOS_PORT               = $(FREERTOS_PORT)"
	@echo "VERSION                     = $(VERSION)"
	@echo "APPCONFIG_HEADER            = $(APPCONFIG_HEADER)"
	@echo "TEST_TIMING                 = $(TEST_TIMING)"
	@echo "USB_CONTROL                 = $(USB_CONTROL)"
	@echo "BTN_CONTROL                 = $(BTN_CONTROL)"
	@echo "ADC_VREF                    = $(ADC_VREF)"
	@echo "H2S_SENSOR_SERIAL_NUMBER    = $(H2S_SENSOR_SERIAL_NUMBER)"
	@echo "H2S_SENSOR_PART_NUMBER      = $(H2S_SENSOR_PART_NUMBER)"
	@echo "H2S_SENSOR_GAS_CODE         = $(H2S_SENSOR_GAS_CODE)"
	@echo "H2S_SENSOR_DATE_CODE        = $(H2S_SENSOR_DATE_CODE)"
	@echo "H2S_SENSOR_SENSITIVITY_CODE = $(H2S_SENSOR_SENSITIVITY_CODE)"

.PHONY: print-build-id
print-build-id: $(TARGET)
	$(TOOLCHAIN)-readelf -n $<

LINT_SCRIPT = uv run scripts/run-clang-format.py
LINT_FLAGS = -r .

.PHONY: lint
lint:
	$(LINT_SCRIPT) $(LINT_FLAGS)

.PHONY: format
format:
	$(LINT_SCRIPT) --in-place $(LINT_FLAGS)

.PHONY: compile-commands
compile-commands: $(BUILD_DIR)/compile_commands.json
$(BUILD_DIR)/compile_commands.json: $(SRC) Makefile scripts/generate-compile-commands.py
	@echo "Generating $@"
	$(Q)scripts/generate-compile-commands.py \
		--objdir=$(OBJDIR) \
		--cflags="-I/usr/include/newlib $(CFLAGS)" \
		--cc=$(CC) \
		$(filter %.c,$^) > $@

TEST_BUILD_DIR = $(BUILD_DIR)/test-build
.PHONY: test
test:
	cmake -B $(TEST_BUILD_DIR) -DCMAKE_BUILD_TYPE=Release test
	cmake --build $(TEST_BUILD_DIR)
	UTEST_COLOR=1 ctest --test-dir $(TEST_BUILD_DIR) --output-on-failure
