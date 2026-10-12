#include "indicator.h"

#include "error.h"

#include <pio.h>
#include <pwm.h>

#include <assert.h>

static pwm_t error_pwm;
static pwm_t status_pwm;

static bool init = false;

#if LED_ACTIVE_HIGH
#define led_off(p) pio_output_low(p)
#define led_on(p) pio_output_high(p)
#else
#define led_off(p) pio_output_high(p)
#define led_on(p) pio_output_low(p)
#endif

static void blink_error(void)
{
    led_off(LED_STATUS_PIO);
    led_on(LED_ERROR_PIO);
    pwm_frequency_set(error_pwm, 6);
    pwm_duty_ppt_set(error_pwm, 500);
    pwm_channels_start(pwm_channel_mask(error_pwm));
}

static void blink_pending(void)
{
    led_off(LED_ERROR_PIO);
    led_on(LED_STATUS_PIO);
}

static void blink_idle(void)
{
    led_off(LED_ERROR_PIO);
    led_on(LED_STATUS_PIO);
    pwm_frequency_set(status_pwm, 2);
    pwm_duty_ppt_set(status_pwm, 500);
    pwm_channels_start(pwm_channel_mask(status_pwm));
}

static void blink_recording(void)
{
    led_off(LED_ERROR_PIO);
    pwm_frequency_set(status_pwm, 6);
    pwm_duty_ppt_set(status_pwm, 500);
    pwm_channels_start(pwm_channel_mask(status_pwm));
}

static void init_pwm(void)
{
    static const pwm_cfg_t error_pwm_cfg = {
        .pio = LED_ERROR_PIO,
        .align = PWM_ALIGN_LEFT,
        .polarity = PWM_POLARITY_HIGH,
        .stop_state = PIO_OUTPUT_LOW,
    };

    static const pwm_cfg_t status_pwm_cfg = {
        .pio = LED_STATUS_PIO,
        .align = PWM_ALIGN_LEFT,
        .polarity = PWM_POLARITY_HIGH,
        .stop_state = PIO_OUTPUT_LOW,
    };

    error_pwm = pwm_init(&error_pwm_cfg);
    if (!error_pwm) {
        error("error initialising PWM for error LED");
    }

    status_pwm = pwm_init(&status_pwm_cfg);
    if (!status_pwm) {
        error("error initialising PWM for status LED");
    }
}

static void (*const blink_funcs[])(void) = {
    [INDICATOR_ERROR] = blink_error,
    [INDICATOR_PENDING] = blink_pending,
    [INDICATOR_RECORDING] = blink_recording,
    [INDICATOR_IDLE] = blink_idle,
};

static_assert(ARRAY_SIZE(blink_funcs) == INDICATOR_NUM_STATES);

void indicator_init(indicator_state_t state)
{
    static error_handler_t error_handler;

    if (init) {
        return;
    }

    error_add_handler(&error_handler, indicator_off);
    init_pwm();
    indicator_set(state);
    init = true;
}

static inline void stop_all_pwm(void)
{
    pwm_channels_stop(
        pwm_channel_mask(status_pwm) | pwm_channel_mask(error_pwm)
    );
}

void indicator_set(indicator_state_t state)
{
    ASSERT(state < INDICATOR_NUM_STATES, "state out of range");
    stop_all_pwm();
    blink_funcs[state]();
}

void indicator_off(void)
{
    stop_all_pwm();
    led_off(LED_ERROR_PIO);
    led_off(LED_STATUS_PIO);
}
