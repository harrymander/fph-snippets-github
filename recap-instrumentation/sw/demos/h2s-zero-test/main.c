#include <delay.h>
#include <pio.h>
#include <usb_serial.h>

#include <h2s.h>

#include <button.h>

#define error(fmt, ...) \
    do { printf(fmt "\n", ##__VA_ARGS__); error_enter(); } while (0)

static void error_enter(void)
{
    pio_config_set(
        LED_ERROR_PIO,
        LED_ACTIVE_HIGH ? PIO_OUTPUT_HIGH : PIO_OUTPUT_LOW
    );
    while (1) {
        pio_output_toggle(LED_ERROR_PIO);
        delay_ms(200);
    }
}

int main(void)
{
    unsigned int led_counter = 0;

    pio_config_set(LED_BLU_PIO, PIO_OUTPUT_HIGH);

    button_init();
    usb_serial_stdio_init();
    if (!h2s_init())
        error("Cannot initialise H2S!");

    printf("H2S initialised!\n");
    while (1) {
        char h2s_buf[H2S_MAX_BUFFER_SIZE];

        delay_ms(10);
        if (button_update(10) && button_read(BUTTON_REC) == BUTTON_DOWN) {
            if (!h2s_zero())
                error("Error zeroing H2S");
            printf("Zeroed H2S\n");
        }

        if (h2s_copy_buffer(h2s_buf, sizeof(h2s_buf)))
            printf("%s\n", h2s_buf);

        if (((led_counter += 10) % 200) == 0)
            pio_output_toggle(LED_BLU_PIO);
    }
}
