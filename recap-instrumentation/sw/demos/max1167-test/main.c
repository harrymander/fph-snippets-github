#include <pio.h>
#include <delay.h>
#include <usb_cdc.h>
#include <target.h>
#include <max1167.h>
#include <tmr.h>
#include <ring.h>
#include <filter.h>


ring_t buffer;

filter_t filter_mask, filter_ear;


void sample(void) {
    uint16_t samples[2];

    filter_update(&filter_mask, max1167_read(0));
    filter_update(&filter_ear, max1167_read(1));

    samples[0] = filter_output(&filter_mask);
    samples[1] = filter_output(&filter_ear);

    ring_write(&buffer, samples, sizeof(samples));
}


int main(void) {
    usb_cdc_cfg_t cdc_cfg = { 0 };
    usb_cdc_t cdc = usb_cdc_init(&cdc_cfg);

    pio_config_set(LED_BLU_PIO, PIO_OUTPUT_LOW);
    pio_config_set(LED_RED_PIO, PIO_OUTPUT_LOW);

    spi_cfg_t cfg = {
        .channel = SPI_CHANNEL0,
        .clock_speed_kHz = 4000,
        .cs = PA11_PIO
    };

    max1167_init(&cfg);

    ring_init(&buffer, NULL, 0xffff);

    filter_init(&filter_ear);
    filter_init(&filter_mask);

    tmr_t timer = tmr_get(0, 0);
    tmr_configure(&timer, TIMER_PRESCALER_2, F_CPU / 2 / 5000);
    tmr_register_irq(&timer, &sample);
    tmr_start(&timer);

    bool sampling = false;
    uint32_t tick = 0;

    while (true) {
        int32_t tmp;
        uint16_t samples[2];

        if (usb_cdc_read_ready_p(cdc)) {
            usb_cdc_getc(cdc);
            sampling = !sampling;
            pio_output_set(LED_RED_PIO, sampling);
        }

        if (RING_READ_NUM(&buffer, tmp) >= sizeof(samples)) {
            ring_read(&buffer, samples, sizeof(samples));

            if (sampling) {
                char str[20];
                snprintf(str, sizeof(str), "%hu,%hu\n", samples[0], samples[1]);
                usb_cdc_puts(cdc, str);
                // fflush(stdout);
            }
        }
    }

    return 0;
}
