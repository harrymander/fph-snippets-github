#pragma once
#include <target.h>

#include "h2s-reading.h"

#include <stdbool.h>
#include <stdint.h>

#define H2S_MAX_BUFFER_SIZE 200

/**
 * @brief Initialize the USART systen and then put the H2S sensor in automatic
 *     mode where it will sample at ~1 Hz and report back to us.
 *
 * @return true if the system was succesfully initialized.
 * @return false if the H2S sensor didn't report back.
 */
bool h2s_init(void);

/**
 * Copy the last character buffer into dest. Copies at most n bytes (including
 * null-terminator). If the whole buffer was copied into dest, returns the
 * number of characters copied (excluding null-terminator).
 *
 * dest must be non-NULL and n > 0
 *
 * If the buffer was truncated, returns the number of characters (excluding the
 * null-terminator) that would have been written if enough space had been
 * available. If no buffer is available since the last call to this function or
 * h2s_read, returns 0.
 *
 * Once this function or h2s_read are called, the last buffer is cleared and
 * subequent calls will return 0 until a new buffer is read.
 */
size_t h2s_copy_buffer(char *dest, size_t n) __attribute__((nonnull));

/**
 * @brief Get the current H2S reading. Should be called regularly.
 *
 * @return bool Returns false if no new H2S reading received since the last time
 *     function called, in which case the value of reading is undefined. Returns
 *     true if a reading was taken, in which case reading will be populated with
 *     the sensor values.
 */
bool h2s_read(struct h2s_reading *reading);

bool h2s_zero(void);
