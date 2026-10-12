#include <pio.h>
#include <delay.h>
#include <usb_serial.h>
#include <target.h>
#include <twi.h>
#include <sdp810.h>

int main(void) {
    usb_cdc_cfg_t cdc_cfg = { 0 };
    usb_serial_init(&cdc_cfg, "/dev/usb");
    freopen("/dev/usb", "w", stdout);

    pio_config_set(LED_BLU_PIO, PIO_OUTPUT_LOW);
    pio_config_set(LED_RED_PIO, PIO_OUTPUT_LOW);

    twi_cfg_t cfg = {
        .channel = TWI_CHANNEL_0,
        .period = TWI_PERIOD_DIVISOR(100000)
    };

    twi_t twi = twi_init(&cfg);

    if (!sdp810_init(twi)) {
        while (true) {
            delay_ms(500);
            pio_output_toggle(LED_RED_PIO);
        }
    }

    while (true) {
        delay_ms(1);
        pio_output_toggle(LED_BLU_PIO);

        float value = sdp810_convert(sdp810_get_pressure());
        printf("%f\n", value);
        fflush(stdout);
    }

    return 0;
}
