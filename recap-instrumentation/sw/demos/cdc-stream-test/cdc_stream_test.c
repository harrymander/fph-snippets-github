#include <target.h>

#include <stdint.h>
#include <string.h>

#include <status.h>

#include <delay.h>
#include <pacer.h>
#include <ring.h>
#include <tmr.h>
#include <usb_cdc.h>
#include <usb_serial.h>


static void error(void)
{
    status_set_mode(STATUS_ERROR);
    while (1) ;
}

static const uint8_t sample_header = 0;
struct sample {
    uint32_t countup;
    int32_t countdown;
    char alphabet;
} __attribute__((packed));

static volatile uint32_t millis;
static volatile bool recording = false;
static ring_t sample_buffer;
#define BUF_SIZE 800

static void sample_timer_callback(void)
{
    static struct sample current_sample = {
        .countup = 0,
        .countdown = INT32_MAX,
        .alphabet = 'A',
    };

    if (recording) {
        ring_write_continuous(&sample_buffer, &current_sample, sizeof(current_sample));
        current_sample.countup += 1;
        current_sample.countdown -= 1;
        current_sample.alphabet = 'A' + ((current_sample.alphabet + 1 - 'A') % 26);
    }
}

static void cdc_send_msg(usb_cdc_t cdc, const char *msg)
{
    size_t msg_size = strlen(msg);
    if (!msg_size)
        error();

    if (msg_size > 0xFF)
        msg_size = 0xFF;

    const uint8_t msg_size_char = msg_size;
    if (usb_cdc_write(cdc, &msg_size_char, 1) != 1)
        error();
    if ((uint8_t) usb_cdc_write(cdc, msg, msg_size) != msg_size_char)
        error();
}

static void send_msg(usb_cdc_t cdc)
{
    static const char * const messages[] = {
        "Hello, World!",
        "Testing USB CDC binary data streaming",
        "The quick brown fox jumps over the lazy dog",
        "This long message, which is testing universial serial bus (USB) "
        "character device class (CDC) streaming on the RECAP (risk "
        "estimation calculator for aspiration pneumonia) instrument board "
        "(prototype version 1), is exactly 255 bytes long... Good-bye, world!"
    };

    static int message_index = 0;
    const char *message = messages[message_index];

    message_index = (message_index + 1) % (sizeof(messages) / sizeof(messages[0]));
    cdc_send_msg(cdc, message);
}

int main(void)
{
    status_init();
    status_set_mode(STATUS_IDLE);

    const usb_cdc_cfg_t cfg = {
        .write_timeout_us = 0,
        .read_timeout_us = 0,
    };
    usb_cdc_t cdc = usb_cdc_init(&cfg);
    if (!cdc)
        error();

    if (!ring_init(&sample_buffer, NULL, sizeof(struct sample) * BUF_SIZE))
        error();

    tmr_t sample_timer = tmr_get(0, 1);
    tmr_configure(&sample_timer, TIMER_PRESCALER_2, F_CPU / 2 / 5000);
    tmr_register_irq(&sample_timer, sample_timer_callback);
    tmr_start(&sample_timer);

    pacer_init(1000);
    while (true) {
        pacer_wait();

        if (usb_cdc_read_ready_p(cdc)) {
            char command;
            if (usb_cdc_read(cdc, &command, 1) != 1)
                error();

            switch (command) {
            case 's':
                recording = true;
                break;
            case 'm':
                if (!recording) {
                    send_msg(cdc);
                    break;
                }
            case 'x':
            default:
                recording = false;
                ring_clear(&sample_buffer);
                break;
            }
        }

        while (!ring_empty_p(&sample_buffer)) {
            struct sample s;
            ring_read(&sample_buffer, &s, sizeof(s));

            if (usb_cdc_write(cdc, &sample_header, 1) != 1)
                error();
            if (usb_cdc_write(cdc, &s, sizeof(s)) != sizeof(s))
                error();
        }
    }
}
