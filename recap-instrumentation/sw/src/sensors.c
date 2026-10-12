#include "sensors.h"

#include "target.h"

#include "error.h"
#include "pio.h"

#include <appconfig.h>
#include <max1167.h>
#include <spi.h>
#include <tmr.h>
#include <twi-async.h>

#include <FreeRTOS.h>
#include <freertos-port.h>
#include <queue.h>

#include <assert.h>
#include <stdint.h>

#if ENABLE_H2S
#include <h2s.h>
#endif

#ifndef TEST_TIMING
#error TEST_TIMING not defined
#endif

static tmr_t *timer_handle;
static twi_async_t *sdp_twi;
static twi_async_t *sfm_twi;
static QueueHandle_t data_queue;

static volatile enum {
    IDLE,
    STARTING_RECORDING,
    RECORDING,
    STOPPING_RECORDING,
} recording_state = IDLE;

/**
 * Interrupt priorities, lower number is higher priority. ISRs that call
 * FreeRTOS API functions must have priority >0.
 */
#define TIMER_IRQ_PRI 1
#define TWI_IRQ_PRI 2

#ifndef EVENT_BTN_DEBOUNCE_PERIOD_MS
#define EVENT_BTN_DEBOUNCE_PERIOD_MS 50
#endif

_Static_assert(
    EVENT_BTN_DEBOUNCE_PERIOD_MS < 0xff, "Debounce period should fit in uint8_t"
);

static inline bool get_event_btn_status(void)
{
#if EVENT_BTN_DEBOUNCE_PERIOD_MS > 0
    static uint8_t ticks = 0;
    static bool status = false;
    const bool pin_status = !pio_input_get(EVENT_BTN_PIO);
    if (pin_status != status) {
        ticks++;
        if (ticks == EVENT_BTN_DEBOUNCE_PERIOD_MS) {
            ticks = 0;
            status = pin_status;
        }
    } else {
        ticks = 0;
    }

    return status;
#else
    return !pio_input_get(EVENT_BTN_PIO);
#endif
}

static inline void read_h2s(struct sensors_sample *sample)
{
#if ENABLE_H2S
    struct h2s_reading h2s_reading;
    if (h2s_read(&h2s_reading)) {
        sample->h2s = h2s_reading.ppb;
        sample->h2s_temp = h2s_reading.celsius;
        sample->h2s_humidity = h2s_reading.humidity;
    } else {
        sample->h2s = INT32_MIN;
    }
#else
    sample->h2s = INT32_MIN;
#endif
}

static inline void
send_sample(const struct sensors_sample *sample, BaseType_t *task_woken)
{
    if (xQueueSendFromISR(data_queue, sample, task_woken) != pdTRUE) {
        error("error writing to sample queue");
    }
}

static void timer_isr(void)
{
    static struct sensors_sample sample;
    static uint8_t ticks = 0;
    static uint32_t millis = 0;
    BaseType_t task_woken = pdFALSE;

    (void) tmr_clear_irq(timer_handle);

#if TEST_TIMING
    pio_output_toggle(TIMING_TEST_PIO);
#endif

    sample.mask = max1167_read(MASK_MIC_ADC_CH);
    sample.ear = max1167_read(EAR_MIC_ADC_CH);
    sample.audio_only = true;

    if (ticks++ == 0) {
        if (sdp810_async_read() < 0) {
            error("error starting async pressure sensor read");
        }
        if (sfm3xxx_async_read() < 0) {
            error("error starting async flow sensor read");
        }
    } else if (ticks == SENSORS_SAMPLE_RATE_KHZ) {
        int r;

        r = sdp810_async_poll(&sample.pressure);
        if (r < 0) {
            error("error reading pressure sensor");
        } else if (r == 0) {
            error("pressure sensor read timeout");
        }

        r = sfm3xxx_async_poll(&sample.flow);
        if (r < 0) {
            error("error reading flow sensor");
        } else if (r == 0) {
            error("flow sensor read timeout");
        }

        sample.millis = millis;
        sample.event = get_event_btn_status();
        sample.audio_only = false;
        read_h2s(&sample);
        ticks = 0;

        // First and stop samples must be sent on the millisecond
        if (recording_state == STARTING_RECORDING) {
            sample.millis = millis = 1;
            recording_state = RECORDING;
        } else if (recording_state == STOPPING_RECORDING) {
            sample.millis = 0;
            recording_state = IDLE;
            send_sample(&sample, &task_woken);
        }

        millis += 1;
    }

    if (recording_state == RECORDING || recording_state == STOPPING_RECORDING) {
        send_sample(&sample, &task_woken);
    }

    portYIELD_FROM_ISR(task_woken);
}

static void handle_error(void)
{
    if (timer_handle) {
        tmr_stop(timer_handle);
    }
}

static void pressure_flow_sensors_init(void)
{
#if (                                                                          \
    SDP810_SFM3XXX_COMBINED_TWI == TWI_CHANNEL_1                               \
    || SDP810_TWI == TWI_CHANNEL_1 || SFM3XXX_TWI == TWI_CHANNEL_1             \
)
    mcu_jtag_disable();
#endif

#ifdef SDP810_SFM3XXX_COMBINED_TWI
    static const twi_cfg_t twi_cfg = {
        .channel = SDP810_SFM3XXX_COMBINED_TWI,
        .period = TWI_PERIOD_DIVISOR(400000)
    };

    sdp_twi = twi_async_init(&twi_cfg, TWI_IRQ_PRI);
    sfm_twi = sdp_twi;
#else
    static const twi_cfg_t sdp_cfg = {
        .channel = SDP810_TWI,
        .period = TWI_PERIOD_DIVISOR(400000),
    };
    static const twi_cfg_t sfm_cfg = {
        .channel = SFM3XXX_TWI,
        .period = TWI_PERIOD_DIVISOR(400000),
    };

    sdp_twi = twi_async_init(&sdp_cfg, TWI_IRQ_PRI);
    sfm_twi = twi_async_init(&sfm_cfg, TWI_IRQ_PRI);
#endif

    if (!sfm3xxx_init(sfm_twi)) {
        error("flow meter initialisation error");
    }
    if (!sdp810_init(sdp_twi)) {
        error("pressure sensor initialisation error");
    }
}

static void mic_adc_init(void)
{
    static const spi_cfg_t max_cfg = {
        .channel = SPI_CHANNEL0, .clock_speed_kHz = 4800, .cs = PA11_PIO
    };
    if (!max1167_init(&max_cfg)) {
        error("ADC initialisation error");
    }
}

static void sample_timer_init(void)
{
    static_assert(
        F_CPU % (2 * SENSORS_SAMPLE_RATE_KHZ * 1000) == 0,
        "Sample rate does not divide into clock frequency"
    );
    timer_handle = tmr_init(
        TMR_MODULE_0,
        TMR_CHANNEL_1,
        TMR_PRESCALER_2,
        F_CPU / 2 / (SENSORS_SAMPLE_RATE_KHZ * 1000),
        TIMER_IRQ_PRI,
        timer_isr
    );
    if (!timer_handle) {
        error("error initialising sample timer");
    }
}

void sensors_init(void)
{
    static error_handler_t error_handler;
    error_add_handler(&error_handler, handle_error);

    DECLARE_STATIC_QUEUE(data_queue, sizeof(struct sensors_sample), 800);
    data_queue = INIT_STATIC_QUEUE(data_queue);
    if (!data_queue) {
        error("error creating sensors data queue");
    }

    // Some initialisation functions use CPU delay functions, which will
    // conflict with the scheduler
    vTaskSuspendAll();

    pressure_flow_sensors_init();

#if ENABLE_H2S
    if (!h2s_init()) {
        error("H2S sensor initialisation error");
    }
#endif

    mic_adc_init();

#if TEST_TIMING
    pio_config_set(TIMING_TEST_PIO, PIO_OUTPUT_LOW);
#endif
    pio_config_set(EVENT_BTN_PIO, PIO_PULLUP);

    sample_timer_init();

    ASSERT(
        !xTaskResumeAll(),
        "context switch should not occur during sensor initialisation"
    );

    tmr_start(timer_handle);
}

void sensors_start_recording(void)
{
    tmr_stop(timer_handle);
    recording_state = STARTING_RECORDING;
    tmr_start(timer_handle);
}

void sensors_stop_recording(void)
{
    tmr_stop(timer_handle);
    recording_state = STOPPING_RECORDING;
    tmr_start(timer_handle);
}

bool sensors_read(struct sensors_sample *data, int timeout)
{
    return xQueueReceive(
               data_queue,
               data,
               timeout < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout)
           )
        == pdTRUE;
}

void sensors_clear_read_buffer(void)
{
    xQueueReset(data_queue);
}
