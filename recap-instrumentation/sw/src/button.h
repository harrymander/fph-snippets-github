#ifndef RECAP_INSTRUMENT_BUTTON_H
#define RECAP_INSTRUMENT_BUTTON_H

#include <stdbool.h>

#ifndef BUTTONS
#define BUTTONS(_) _(REC, RECORD_BTN_PIO, false)
#endif

#define BUTTONS_ENUM(name, ...) BUTTON_##name,

typedef enum {
    BUTTONS(BUTTONS_ENUM) NUM_BUTTONS
} button_t;

#undef BUTTONS_ENUM

typedef enum {
    BUTTON_NOCHANGE = -1,
    BUTTON_UP = 0,
    BUTTON_DOWN = 1,
} button_state_t;

void button_init(void);

// Returns true if any button events occurred
bool button_update(unsigned int millis);

button_state_t button_read(button_t button);

#endif // RECAP_INSTRUMENT_BUTTON_H
