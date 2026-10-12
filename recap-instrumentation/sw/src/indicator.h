#ifndef RECAP_INSTRUMENT_INDICATOR_H
#define RECAP_INSTRUMENT_INDICATOR_H

typedef enum {
    INDICATOR_ERROR = 0,
    INDICATOR_PENDING,
    INDICATOR_IDLE,
    INDICATOR_RECORDING,
    INDICATOR_NUM_STATES,
} indicator_state_t;

void indicator_init(indicator_state_t state);
void indicator_set(indicator_state_t state);
void indicator_off(void);

#endif // RECAP_INSTRUMENT_INDICATOR_H
