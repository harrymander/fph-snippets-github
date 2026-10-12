#include <target.h>

#include <FreeRTOS.h>
#include <task.h>

#include <pio.h>
#include <delay.h>

#include <freertos-port.h>

#define error() vAssertCalled(__FILE__, __LINE__)

struct arg {
    unsigned int pio;
    TickType_t ticks;
};

static void task(void *argptr)
{
    const struct arg *arg = argptr;
    TickType_t wake_time = xTaskGetTickCount();
    while (1) {
        pio_output_toggle(arg->pio);
        vTaskDelayUntil(&wake_time, arg->ticks);
    }
}

int main(void)
{
    freertos_port_init();

    pio_config_set(EVENT_BTN_PIO, PIO_OUTPUT_LOW);
    pio_config_set(MANUAL_PIO, PIO_OUTPUT_LOW);

    static const struct arg args[] = {
        { .pio = EVENT_BTN_PIO, .ticks = pdMS_TO_TICKS(1) },
        { .pio = MANUAL_PIO, .ticks = pdMS_TO_TICKS(10) },
    };

    DECLARE_STATIC_TASK(task1, 128);
    if (!INIT_STATIC_TASK(task1, task, "task1", (void *) &args[0], 1))
        error();

    DECLARE_STATIC_TASK(task2, 128);
    if (!INIT_STATIC_TASK(task2, task, "task2", (void *) &args[1], 0))
        error();

    vTaskStartScheduler();

    return 0;
}

void vAssertCalled(const char *file, unsigned long line)
{
    pio_config_set(LED_RED_PIO, PIO_OUTPUT_LOW);
    while (1) {
        pio_output_toggle(LED_RED_PIO);
        delay_ms(200);
    }
}
