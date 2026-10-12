#include <delay.h>
#include <pio.h>
#include <usb_cdc.h>
#include <usart.h>

#include <h2s.h>

static void error(void)
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
    usart_cfg_t usart_cfg = {
        .channel = 0,
        .baud_rate = 9600,
        .write_timeout_us = 50000,
    };
    usb_cdc_cfg_t usb_cfg = {0}; // no timeout
    usart_t h2s_usart;
    usb_cdc_t usb_cdc;

    pio_config_set(LED_BLU_PIO, PIO_OUTPUT_HIGH);
    usb_cdc = usb_cdc_init(&usb_cfg);
    if (!usb_cdc)
        error();
    h2s_usart = usart_init(&usart_cfg);
    if (!h2s_usart)
        error();

    while (1) {

        if (usb_cdc_read_ready_p(usb_cdc)) {
            char c;
            usb_cdc_read(usb_cdc, &c, 1);
            if (c == 'B') {
                usb_cdc_puts(usb_cdc, "Writing barcode\n");
                usart_write(h2s_usart, "B", 1);
                delay_ms(1000);
                const char *barcode = "072116020232 110303 H2S 1608 287.59\r\n";
                while (*barcode) {
                    usart_write(h2s_usart, barcode, 1);
                    delay_ms(20);
                    barcode++;
                }
                usb_cdc_puts(usb_cdc, "Wrote barcode\n");
            } else {
                usart_write(h2s_usart, &c, 1);
            }
        }

        if (usart_read_ready_p(h2s_usart)) {
            const char c = usart_getc(h2s_usart);
            usb_cdc_write(usb_cdc, &c, 1);
        }
    }
}
