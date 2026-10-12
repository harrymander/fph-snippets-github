#include <pio.h>
#include <delay.h>

#include <target.h>


int main(void) {

    pio_config_set(LED1_PIO, PIO_OUTPUT_HIGH);
    pio_config_set(LED2_PIO, PIO_OUTPUT_LOW);

    while (true) {

        delay_ms(500);
        pio_output_toggle(LED1_PIO);
        pio_output_toggle(LED2_PIO);
    }

    return 0;
}
