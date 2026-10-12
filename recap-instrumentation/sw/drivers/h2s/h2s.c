/**
 * @file h2s.c
 * @author Ben Mitchell (benjamin.mitchell@canterbury.ac.nz)
 * @brief Device driver for the Spec Sensors digital H2S gas sensor. This module
 * handles initializing the UART communications and processing the sensors
 * serial communications to report the current H2S level in PPB.
 * @version 0.1
 * @date 2021-12-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "h2s.h"

#include "target.h"

#include "delay.h"
#include "irq.h"
#include "pio.h"
#include "usart.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define H2S_INIT_TIMEOUT 4000
#define H2S_INIT_RETRIES 3
#define H2S_NUM_BUFFERS 2

#define H2S_ZERO_TIMEOUT 2000

#define ZERO_MSG "Setting zero...done"

static usart_t h2s;
static char buffers[H2S_MAX_BUFFER_SIZE * H2S_NUM_BUFFERS];
static char *volatile prev_buffer = NULL;

/**
 * @brief USART character received IRQ. This will:
 * a) buffer the recieved character.
 * b) flag if the H2S sensor has finished sending a line and there is new data.
 *
 * If the buffer is full, then simply clear it and try again.
 */
void h2s_irq(void)
{
    static int pos = 0;
    static char *buffer = &buffers[0];

    irq_clear(USART0_IRQn);

    const int c = usart_getc(h2s);
    if (c == '\n' || c == '\r') {
        // \n and \r occur sequentially so this won't be an issue
        if (pos > 0) {
            static int buffer_index = 0;
            buffer[pos] = '\0';
            pos = 0;
            prev_buffer = buffer;
            buffer_index = (buffer_index + 1) % H2S_NUM_BUFFERS;
            buffer = &buffers[buffer_index * H2S_MAX_BUFFER_SIZE];
        }
    } else {
        buffer[pos++] = c;

        // edge case: the data hasn't been read yet and we are now bashing it.
        if (pos == H2S_MAX_BUFFER_SIZE) {
            pos = 0;
        }
    }
}

/**
 * @brief Initialize the USART systen and then put the H2S sensor in automatic
 *     mode where it will sample at ~1 Hz and report back to us.
 *
 * @return true if the system was succesfully initialized.
 * @return false if the H2S sensor didn't report back.
 */
bool h2s_init(void)
{
    usart_cfg_t cfg = {
        .channel = 0,
        .baud_rate = 9600,
        .write_timeout_us = 50000,
    };

    int retries = H2S_INIT_RETRIES;

    h2s = usart_init(&cfg);

    // HACK: enable IRQ for recieved bytes
    USART0->US_IER = US_IER_RXRDY;

    // setup the IRQ to handle future bytes
    irq_config(USART0_IRQn, 4, &h2s_irq);
    irq_enable(USART0_IRQn);

    while (retries) {
        int timeout = H2S_INIT_TIMEOUT;

        usart_putc(h2s, 'c');
        delay_ms(500);
        usart_putc(h2s, 'c');

        // wait for an incoming H2S reading (or a timeout)
        while (timeout > 0) {
            struct h2s_reading reading;
            if (h2s_read(&reading)) {
                return true;
            }
            delay_ms(100);
            timeout -= 100;
        }

        retries -= 1;
    }

    errno = ENODEV;
    irq_disable(USART0_IRQn);
    return false;
}

static inline char *take_buffer(void)
{
    irq_disable(USART0_IRQn);
    char *const buf = prev_buffer;
    if (!buf) {
        irq_enable(USART0_IRQn);
    }
    return buf;
}

static inline void release_buffer(void)
{
    prev_buffer = NULL;
    irq_enable(USART0_IRQn);
}

size_t h2s_copy_buffer(char *dest, size_t n)
{
    size_t size = 0;
    char *buf = take_buffer();
    if (buf) {
        while (*buf && size < n - 1) {
            dest[size] = *buf;
            buf++;
            size++;
        }

        dest[size] = '\0';

        // Read the rest of the buffer
        while (*buf) {
            buf++;
            size++;
        }

        release_buffer();
    }

    return size;
}

bool h2s_read(struct h2s_reading *reading)
{
    char *buf = take_buffer();
    if (buf) {
        const bool valid = h2s_reading_parse(buf, reading);
        release_buffer();
        return valid;
    }

    return false;
}

bool h2s_zero(void)
{
    usart_putc(h2s, 'Z');

    unsigned int timeout = 0;
    while (timeout < H2S_ZERO_TIMEOUT) {
        char buf[H2S_MAX_BUFFER_SIZE];
        if (h2s_copy_buffer(buf, sizeof(buf))
            && strncmp(buf, ZERO_MSG, strlen(ZERO_MSG)) == 0)
        {
            return true;
        }

        delay_ms(10);
        timeout += 10;
    }

    return false;
}
