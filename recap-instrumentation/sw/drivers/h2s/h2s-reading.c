#include "h2s-reading.h"

#include <appconfig.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define H2S_SERIAL_PREFIX H2S_SENSOR_SERIAL_NUMBER ","

_Static_assert(
    sizeof(long) >= sizeof(int32_t), "sizeof(long) < sizeof(int32_t)"
);

// Returns INT32_MIN on error
static inline int32_t parseint(const char *str, char **endptr)
{
    const long tmp = strtol(str, endptr, 10);
    if (*endptr != str     // No parsing error
        && tmp > INT32_MIN // No underflow
        && tmp < INT32_MAX // No overflow
    )
    {
        return tmp;
    }
    return INT32_MIN;
}

bool h2s_reading_parse(char *str, struct h2s_reading *reading)
{
    // Check for serial number followed by a comma
    if (strncmp(str, H2S_SERIAL_PREFIX, sizeof(H2S_SERIAL_PREFIX) - 1)) {
        return false;
    }
    str += sizeof(H2S_SERIAL_PREFIX) - 1;
    if (!(*str)) {
        return false;
    }

    // Read ppb followed by a comma
    reading->ppb = parseint(str, &str);
    if (reading->ppb == INT32_MIN || *str != ',') {
        return false;
    }

    // Read the temperature followed by a comma
    str += 1;
    if (!(*str)) {
        return false;
    }
    const int32_t celsius = parseint(str, &str);
    if (celsius < INT8_MIN || celsius > INT8_MAX || *str != ',') {
        return false;
    }
    reading->celsius = celsius;

    // Read the humidity followed by a comma or end of string
    str += 1;
    if (!(*str)) {
        return false;
    }
    const int32_t humidity = parseint(str, &str);
    if (humidity < 0 || humidity > 100) {
        return false;
    }
    reading->humidity = humidity;
    return !(*str) || *str == ',';
}
