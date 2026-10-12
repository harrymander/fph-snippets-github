#ifndef RECAP_INSTRUMENT_RECORD_H
#define RECAP_INSTRUMENT_RECORD_H

#include <stdbool.h>

void record_init(void);

// Return false if timed-out
bool record_start(int timeout);
bool record_stop(int timeout);

#endif // RECAP_INSTRUMENT_RECORD_H
