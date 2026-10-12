#ifndef RECAP_H2S_READING_H
#define RECAP_H2S_READING_H

#include <stdbool.h>
#include <stdint.h>

struct h2s_reading {
    int32_t ppb;
    int8_t celsius;
    uint8_t humidity;
};

/**
 * Returns true if the reading is valid and populates fields of reading. If
 * invalid, returns false and the value of reading is undefined. Undefined
 * behaviour if either argument is NULL.
 */
bool h2s_reading_parse(char *str, struct h2s_reading *reading);

#endif // RECAP_H2S_READING_H
