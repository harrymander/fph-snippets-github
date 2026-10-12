/**
 * @file status.c
 * @author Ben Mitchell (benjamin.canterbury.ac.nz)
 * @brief Helper module to automate the status LED behaviour. Supports three
 * modes:
 *  - INIT: solid blue LED
 *  - IDLE: blue LED blinking slowly.
 *  - RECORDING: blue LED blinking fast with red LED on solid.
 *  - ERROR: red LED blinking fast.
 * @version 0.1
 * @date 2021-12-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "status.h"

#include <target.h>

#include "tmr.h"

#include <irq.h>
#include <pio.h>

static volatile status_t mode = STATUS_INIT;
static uint32_t tick = 0;

/**
 * @brief Invoked by the timer module, updates the state of the LEDs based on
 * mode.
 */
void status_irq(void)
{
    switch (mode) {
    case STATUS_INIT:
        // Solid blue LED
        pio_output_high(LED_BLU_PIO);
        break;

    case STATUS_IDLE:
        // blink the blue LED slowly
        pio_output_set(LED_BLU_PIO, ((++tick) / 4) & 1);
        break;

    case STATUS_RECORDING:
        // blink the blue LED quickly
        pio_output_toggle(LED_BLU_PIO);
        break;

    case STATUS_ERROR:
        // Do blinking inside the error function
        break;
    }
}

void status_init(void)
{
    pio_config_set(LED_BLU_PIO, PIO_OUTPUT_LOW);
    pio_config_set(LED_RED_PIO, PIO_OUTPUT_LOW);

    // setup the timer for 12 Hz interrupts (about as slow as we can go with 16
    // bit timers and a 128 prescaler).
    tmr_t tmr = tmr_get(0, 0);
    tmr_configure(&tmr, TIMER_PRESCALER_128, F_CPU / 128 / 12);
    tmr_register_irq(&tmr, &status_irq);
    tmr_start(&tmr);
}

void status_set_mode(status_t m)
{
    mode = m;
    tick = 0; // restart the pattern

    // force the LEDs to a known state (off) for the new mode
    pio_output_low(LED_BLU_PIO);
    pio_output_low(LED_RED_PIO);
}

status_t status_get_mode(void)
{
    return mode;
}
