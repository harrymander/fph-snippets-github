/*
 * Simple program for testing input/output via USB serial
 */

#include <pio.h>
#include <delay.h>
#include <usb_serial.h>
#include <target.h>


int main(void)
{
    usb_serial_stdio_init();

    pio_config_set(LED_BLU_PIO, PIO_OUTPUT_HIGH);

    puts("> ");
    while (true) {
        char buf[256];
        if (fgets(buf, sizeof(buf), stdin)) {
            int n;
            if (sscanf(buf, "%d", &n) == 1) {
                pio_output_toggle(LED_BLU_PIO);
                printf("%d\n", 2 * n);
                puts("> ");
            }
        }
    }

    return 0;
}
