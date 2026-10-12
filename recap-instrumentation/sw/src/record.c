#include "record.h"

#include "target.h"

#include "build-id.h"
#include "error.h"
#include "priorities.h"
#include "sensors.h"
#include "usbio.h"

#include <appconfig.h>

#include <FreeRTOS.h>
#include <event_groups.h>
#include <freertos-port.h>
#include <task.h>

#include <stdbool.h>
#include <stdint.h>

#define STOP_BIT (1 << 2)
#define START_BIT (1 << 0)

static bool recording = false;

// Notify task initialisation complete
static EventGroupHandle_t ready_egroup;

// Start/stop recording
static EventGroupHandle_t ctrl_egroup;

// Notify caller recording has started/stopped
static EventGroupHandle_t sync_egroup;

static void new_record(void)
{
    static uint8_t record_version[2] = {5, 0};
    static const char record_signature[] = "-RECAP-";

    if (usbio_write(record_signature, sizeof(record_signature) - 1)
        != sizeof(record_signature) - 1)
    {
        error("error writing record signature");
    }

    if (usbio_write(record_version, sizeof(record_version))
        != sizeof(record_version))
    {
        error("error writing record version");
    }

    for (uint8_t i = 0; i < 2; i++) {
        if (usbio_write(&(uint64_t) {0}, sizeof(uint64_t)) != sizeof(uint64_t))
        {
            error("error writing file size and field");
        }
    }

    if (usbio_printf(
            "Firmware: " VERSION_STR "\n"
            "Build: %s\n"
            "Sample-Rate: %u\n"
            "VREF: " ADC_VREF_MILLIVOLTS_STR "\n"
            "Board: " RECAP_BOARD_NAME "\n"
            "Flow: " SFM3XXX_DEV_NAME "\n"
            "Flow-Offset: " SFM3XXX_HEADER_OFFSET "\n"
            "Flow-Scale: " SFM3XXX_HEADER_SCALE "\n"
            "Flow-Sign: " SFM3XXX_HEADER_SIGNED "\n"
            "H2S-Barcode: " H2S_SENSOR_BARCODE "\n"
            "H2S-Enabled: %hhu\n"
            "Test-Timing: %hhu\n",
            build_id_digest_str(),
            SENSORS_SAMPLE_RATE_KHZ,
            (bool) ENABLE_H2S,
            (bool) TEST_TIMING
        )
        < 0)
    {
        error("error writing record header");
    }

    if (usbio_write(&(char) {0}, 1) != 1) {
        error("error writing null terminator");
    }
}

static void write_sample(const struct sensors_sample *sample)
{
#define WRITE_FIELD(field)                                                     \
    if (usbio_write(&sample->field, sizeof(sample->field))                     \
        != sizeof(sample->field))                                              \
    {                                                                          \
        error("error writing '" #field "' field");                             \
    }

    if (sample->audio_only) {
        WRITE_FIELD(ear);
        WRITE_FIELD(mask);
    } else {
        WRITE_FIELD(millis);
        WRITE_FIELD(flow);
        WRITE_FIELD(pressure);
        WRITE_FIELD(ear);
        WRITE_FIELD(mask);
        WRITE_FIELD(event);
        WRITE_FIELD(h2s);
        WRITE_FIELD(h2s_temp);
        WRITE_FIELD(h2s_humidity);
    }
#undef WRITE_FIELD
}

static inline EventBits_t wait_sync_bits(EventBits_t wait, TickType_t ticks)
{
    return xEventGroupWaitBits(sync_egroup, wait, pdTRUE, pdTRUE, ticks);
}

static void task(void *arg)
{
    (void) arg;

    sensors_init();
    usbio_init();
    xEventGroupSetBits(ready_egroup, 0x01);

    while (1) {
        if (recording) {
            while (1) {
                struct sensors_sample data;
                sensors_read(&data, -1);
                write_sample(&data);
                if (!data.audio_only && data.millis == 0) {
                    break;
                }
            }

            sensors_clear_read_buffer();
            recording = false;
        } else {
            xEventGroupWaitBits(
                ctrl_egroup, START_BIT, pdTRUE, pdFALSE, portMAX_DELAY
            );
            new_record();
            sensors_start_recording();
            recording = true;
        }

        xEventGroupSetBits(sync_egroup, recording ? START_BIT : STOP_BIT);
    }
}

void record_init(void)
{
    static StaticEventGroup_t ready_egroup_buffer;
    static StaticEventGroup_t sync_egroup_buffer;
    static StaticEventGroup_t ctrl_egroup_buffer;

    ready_egroup = xEventGroupCreateStatic(&ready_egroup_buffer);
    if (!ready_egroup) {
        error("error creating record ready event group");
    }

    sync_egroup = xEventGroupCreateStatic(&sync_egroup_buffer);
    if (!sync_egroup) {
        error("error creating record sync event group");
    }

    ctrl_egroup = xEventGroupCreateStatic(&ctrl_egroup_buffer);
    if (!ctrl_egroup) {
        error("error creating record control event group");
    }

    DECLARE_STATIC_TASK(task, 512);
    if (!INIT_STATIC_TASK(task, task, "record", NULL, PRI_RECORD)) {
        error("error creating record task");
    }

    xEventGroupWaitBits(ready_egroup, 0x01, pdFALSE, pdTRUE, portMAX_DELAY);
}

static TickType_t timeout_ticks(int timeout_millis)
{
    return timeout_millis < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_millis);
}

bool record_start(int timeout)
{
    ASSERT(
        !recording, "record start must not be called when already recording"
    );

    xEventGroupSetBits(ctrl_egroup, START_BIT);

    return START_BIT == wait_sync_bits(START_BIT, timeout_ticks(timeout));
}

bool record_stop(int timeout)
{
    ASSERT(recording, "record stop must not be called when not recording");

    /*
       Because the record task will be blocked reading from the sensor queue, we
       need to tell the sensor task to stop recording and send an
       end-of-recording message to the record task, which will finish writing
       out all the data to the queue and then set the stop bit in sync_egroup.
    */
    sensors_stop_recording();
    return STOP_BIT == wait_sync_bits(STOP_BIT, timeout_ticks(timeout));
}
