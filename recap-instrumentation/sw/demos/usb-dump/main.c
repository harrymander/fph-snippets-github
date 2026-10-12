#include <pio.h>
#include <delay.h>
#include <usb_serial.h>
#include <target.h>


int main(void) {
    usb_cdc_cfg_t cdc_cfg = { 0 };
    usb_serial_init(&cdc_cfg, "/dev/usb");
    freopen("/dev/usb", "w", stdout);
    freopen("/dev/usb", "r", stdin);

    pio_config_set(LED_BLU_PIO, PIO_OUTPUT_HIGH);

    while (true) {
        pio_output_toggle(LED_BLU_PIO);

        getchar();
        for (int i = 0; i < 256; ++i) {
            putchar(i);
        }
    }

    return 0;
}
