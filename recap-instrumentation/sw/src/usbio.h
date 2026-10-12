#ifndef RECAP_INSTRUMENT_USBIO_H_INCLUDE
#define RECAP_INSTRUMENT_USBIO_H_INCLUDE

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Initialise thread safe USB CDC interface. Panics on error
 */
void usbio_init(void);

/**
 * Returns true if a char was read, in which case the character is stored in
 * `c`. Panics on error.
 */
bool usbio_read_char_nonblocking(char *c);

/**
 * Write to USB.
 *
 * Returns number of bytes written, or negative integer on error.
 */
ssize_t usbio_write(const void *data, size_t size);

/**
 * Formatted printing to USB interface.
 *
 * Follows standard printf rules, will panic on IO error, but should still check
 * the return value as a formatting error can occur.
 */
int usbio_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#endif // RECAP_INSTRUMENT_USBIO_H_INCLUDE
