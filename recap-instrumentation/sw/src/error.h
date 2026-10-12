#ifndef RECAP_INSTRUMENT_ERROR_H
#define RECAP_INSTRUMENT_ERROR_H

#include <errno.h>

typedef struct error_handler error_handler_t;

/**
 * Check the reason for reset and enter error state if device not reset from a
 * power-up or by de-asserting NRST. This is needed because it seems that the
 * OpenOCD debugger doesn't properly reset the device after programming, leaving
 * the peripherals in an unknown state leading to all kinds of weird bugs.
 */
void error_check_reset_reason(void);

/**
 * The fields of this struct should be considered 'private' and don't need to be
 * set manually as they are set in `error_add_handler`.
 */
struct error_handler {
    error_handler_t *next;
    void (*handler)(void);
};

/**
 * Register a function to be called when the error state is entered. The
 * lifetime of the `error_handler_t` handle that `handle` points to must exist
 * when the error state is entered (i.e. it should be global/static).
 *
 * Handlers are called in the order that they were registered.
 */
void error_add_handler(error_handler_t *handle, void (*handler)(void));

__attribute__((noreturn)) void application_error(
    const char *message, const char *fname, unsigned int line, int cerrno
);

#define error(m) application_error(m, __FILE__, __LINE__, errno)
#define ASSERT(c, m)                                                           \
    do {                                                                       \
        if (!(c))                                                              \
            error("assertion failed: " m);                                     \
    } while (0)

#endif // RECAP_INSTRUMENT_ERROR_H
