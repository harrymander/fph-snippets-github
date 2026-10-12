#include "control.h"

#include "build-id.h"
#include "error.h"
#include "indicator.h"
#include "priorities.h"
#include "record.h"

#include <appconfig.h>
#include <pio.h>

#include <FreeRTOS.h>
#include <freertos-port.h>
#include <task.h>

#if ENABLE_H2S
#include <h2s.h>
#endif

#if BTN_CONTROL
#include "button.h"
#endif

#if USB_CONTROL
#include "usbio.h"
#endif

#include <string.h>

typedef enum {
    STATE_NO_CHANGE = -1,
    STATE_IDLE = 0,
    STATE_RECORDING,
    NUM_STATES,
} state_t;

#if USB_CONTROL

#define PRINT(fmt, ...)                                                        \
    do {                                                                       \
        if (usbio_printf(fmt, ##__VA_ARGS__) < 0)                              \
            error("printf error");                                             \
    } while (0);

static state_t handle_idle_usb_cmd(void)
{
    char cmd;
    if (usbio_read_char_nonblocking(&cmd)) {
        switch (cmd) {
        case 's':
            return STATE_RECORDING;
        case 'v':
            PRINT(VERSION_STR "\n");
            break;
        case 'b':
            PRINT("%s\n", build_id_digest_str());
            break;
        default:
            break;
        }
    }

    return STATE_NO_CHANGE;
}
#endif

static state_t idle_handler(TickType_t *wake_time)
{
    indicator_set(INDICATOR_IDLE);
    while (1) {
        vTaskDelayUntil(wake_time, pdMS_TO_TICKS(10));

#if BTN_CONTROL
        if (button_update(10) && button_read(BUTTON_REC) == BUTTON_DOWN) {
            return STATE_RECORDING;
        }
#endif
#if USB_CONTROL
        const state_t new_state = handle_idle_usb_cmd();
        if (new_state >= 0) {
            return new_state;
        }
#endif
    }
}

static void start_recording(void)
{
    indicator_set(INDICATOR_PENDING);
    if (!record_start(1000)) {
        error("timed-out starting recording");
    }
    indicator_set(INDICATOR_RECORDING);
}

static void stop_recording(void)
{
    indicator_set(INDICATOR_PENDING);
    if (!record_stop(1000)) {
        error("timed-out stopping recording");
    }
    indicator_set(INDICATOR_IDLE);
}

static void zero_h2s(void)
{
#if ENABLE_H2S
    vTaskSuspendAll();
    if (!h2s_zero()) {
        error("error zeroing H2S sensor");
    }
    xTaskResumeAll();
#endif
}

static bool handle_recording_input(unsigned int millis)
{
#if BTN_CONTROL
    if (button_update(millis) && button_read(BUTTON_REC) == BUTTON_DOWN) {
        return true;
    }
#endif
#if USB_CONTROL
    char cmd;
    if (usbio_read_char_nonblocking(&cmd)) {
        switch (cmd) {
        case 'x':
            return true;
        case 'z':
            zero_h2s();
            break;
        default:
            break;
        }
    }
#endif
    return false;
}

static state_t recording_handler(TickType_t *wake_time)
{
    start_recording();
    while (1) {
        vTaskDelayUntil(wake_time, pdMS_TO_TICKS(10));
        if (handle_recording_input(10)) {
            break;
        }
    }
    stop_recording();
    return STATE_IDLE;
}

static state_t (*const state_handlers[NUM_STATES])(TickType_t *) = {
    [STATE_IDLE] = idle_handler,
    [STATE_RECORDING] = recording_handler,
};

static void task(void *arg)
{
    (void) arg;
    TickType_t wake_time = xTaskGetTickCount();
    indicator_init(INDICATOR_PENDING);
    record_init();
#if BTN_CONTROL
    button_init();
#endif
#if USB_CONTROL
    usbio_init();
#endif
    state_t current_state = STATE_IDLE;
    while (1) {
        current_state = state_handlers[current_state](&wake_time);
    }
}

void control_init(void)
{
    build_id_init();
    DECLARE_STATIC_TASK(task, 128);
    if (!INIT_STATIC_TASK(task, task, "control", NULL, PRI_CONTROL)) {
        error("error creating control task");
    }
}
