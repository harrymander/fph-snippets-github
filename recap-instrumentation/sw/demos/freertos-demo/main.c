#include <target.h>

#include <FreeRTOS.h>
#include <task.h>

#include <pio.h>
#include <delay.h>
#include <queue.h>

#include <freertos-port.h>

#define error() vAssertCalled(__FILE__, __LINE__)

QueueHandle_t led_queue;
QueueHandle_t button_queue;


static void buttons(void *args)
{
    (void) args;

    TickType_t wake_time = xTaskGetTickCount();
    unsigned int counts = 0;
    bool status = 0;
    pio_config_set(MANUAL_PIO, PIO_PULLUP);
    while (true) {
        if ((!pio_input_get(MANUAL_PIO)) == status)
            counts = 0;
        else
            counts += 1;

        if (counts == 5) {
            status = !status;
            xQueueSend(button_queue, &status, portMAX_DELAY);
        }

        vTaskDelayUntil(&wake_time, pdMS_TO_TICKS(10));
    }
}

static void controller(void *args)
{
    (void) args;

    static const int delays[] = { 50, 100, 200, 500, 1000 };
    unsigned int delay_index = 0;

    while (true) {
        bool status;
        if (
            xQueueReceive(button_queue, &status, portMAX_DELAY) == pdTRUE
            && status
        ) {
            xQueueSend(led_queue, delays + delay_index, portMAX_DELAY);
            delay_index = (delay_index + 1) % ARRAY_SIZE(delays);
        }
    }
}

static void blink(void *args)
{
    (void) args;

    TickType_t wake_time = xTaskGetTickCount();
    unsigned int delay_time = portMAX_DELAY;

    pio_config_set(LED_BLU_PIO, PIO_OUTPUT_LOW);

    while (true) {
        if (xQueueReceive(led_queue, &delay_time, delay_time) == pdTRUE)
            delay_time = pdMS_TO_TICKS(delay_time);
        pio_output_toggle(LED_BLU_PIO);
    }
}

int main(void)
{
    freertos_port_init();

    pio_config_set(LED_RED_PIO, PIO_OUTPUT_LOW);

    led_queue = xQueueCreate(1, sizeof(unsigned int));
    if (!led_queue)
        error();

    button_queue = xQueueCreate(1, sizeof(bool));
    if (!button_queue)
        error();

    if (xTaskCreate(&buttons, "buttons", 256, NULL, 0, NULL) != pdPASS)
        error();
    if (xTaskCreate(&controller, "controller", 256, NULL, 0, NULL) != pdPASS)
        error();
    if (xTaskCreate(&blink, "blink", 256, NULL, 0, NULL) != pdPASS)
        error();

    vTaskStartScheduler();

    return 0;
}

void vAssertCalled(const char *file, unsigned long line)
{
    while (1) {
        pio_output_toggle(LED_RED_PIO);
        delay_ms(200);
    }
}
