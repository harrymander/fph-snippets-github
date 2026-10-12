#include "error.h"

#include <target.h>

#include <appconfig.h>
#include <delay.h>
#include <pio.h>
#include <pwm.h>
#include <sys/types.h>

#include <FreeRTOS.h>
#include <task.h>

static struct error_handler *handler_head = NULL;
static struct error_handler *handler_tail = NULL;

// Don't optimise functions: allows us to see error details in backtrace
#define ATTR_NOOPT __attribute__((optimize("O0")))

#define ERROR_LED_FLASH_PERIOD_MS 400

typedef enum {
    APPLICATION_ERROR = 0,
    FREERTOS_ASSERTION_FAILED = 2,
    FREERTOS_STACK_OVERFLOW,
    SBRK_CALLED,
    REQUIRES_HARD_RESET,
    EXIT_CALLED,
} error_code_t;

void error_add_handler(error_handler_t *handle, void (*handler)(void))
{
    handle->handler = handler;
    handle->next = NULL;

    if (handler_tail) {
        handler_tail->next = handle;
        handler_tail = handle;
    } else {
        handler_head = handle;
        handler_tail = handle;
    }
}

static void ATTR_NOOPT __attribute__((noreturn)) error_enter(error_code_t code)
{
    struct error_handler *handler = handler_head;

    vTaskSuspendAll();

    while (handler) {
        handler->handler();
        handler = handler->next;
    }

    pio_config_set(LED_RED_PIO, PIO_OUTPUT_HIGH);

    if (code) {
        while (1) {
            for (unsigned int i = 0; i < code; i++) {
                pio_output_toggle(LED_RED_PIO);
                delay_ms(ERROR_LED_FLASH_PERIOD_MS);
                pio_output_toggle(LED_RED_PIO);
                delay_ms(ERROR_LED_FLASH_PERIOD_MS);
            }

            delay_ms(ERROR_LED_FLASH_PERIOD_MS * 2);
        }
    } else {
        while (1) {
            pio_output_toggle(LED_RED_PIO);
            delay_ms(ERROR_LED_FLASH_PERIOD_MS);
        }
    }
}

void ATTR_NOOPT application_error(
    const char *message, const char *fname, unsigned int line, int cerrno
)
{
    error_enter(APPLICATION_ERROR);
}

/**
 * Declared in FreeRTOSConfig.h, called by FreeRTOS library functions to trap
 * incorrect FreeRTOS usage.
 *
 * See https://www.freertos.org/a00110.html
 */
void ATTR_NOOPT freertos_assert_called(const char *file, unsigned long line)
{
    error_enter(FREERTOS_ASSERTION_FAILED);
}

/**
 * Declared in FreeRTOS task.h. Called when a stack overflow is detected in a
 * task.
 *
 * See https://www.freertos.org/Stacks-and-stack-overflow-checking.html
 */
void ATTR_NOOPT
vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    error_enter(FREERTOS_STACK_OVERFLOW);
}

void ATTR_NOOPT error_check_reset_reason(void)
{
    const uint8_t reset_type = mcu_reset_type_get();
    // Needs to be general/power-up reset (0) or user reset (4)
    if (!(reset_type == 0 || reset_type == 4)) {
        error_enter(REQUIRES_HARD_RESET);
    }
}

/**
 * Wraps the newlib _sbrk syscall, which is used for dynamic memory allocation.
 * Wrapping is configured via -Wl,--wrap linker flag in Makefile. The original
 * function can be called by calling __real__sbrk. See ld(1) for more info on
 * linker wrapping.
 *
 * This is done to ensure that no dynamic memory allocation is performed, as I
 * am not sure how it interacts with FreeRTOS.
 */
caddr_t ATTR_NOOPT __wrap__sbrk(int incr)
{
    (void) incr;
    error_enter(SBRK_CALLED);
}

/**
 * Hook into _exit syscall that is called on stdlib abort/exit/assert calls: see
 * above.
 *
 * The original function is defined in mat91lib/syscalls.c and just enters an
 * infinite loop.
 */
void ATTR_NOOPT __wrap__exit(int status)
{
    (void) status;
    error_enter(EXIT_CALLED);
}
