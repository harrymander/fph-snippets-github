#include "button.h"

#include <target.h>

#include <pio.h>

#define DEBOUNCE_PERIOD 50

static bool init = false;

#define BUTTONS_STRUCT(name, pio_, active_high_)                               \
    [BUTTON_##name] = {                                                        \
        .pio = pio_,                                                           \
        .active_high = active_high_,                                           \
        .millis = 0,                                                           \
        .raw_state = false,                                                    \
        .state = BUTTON_NOCHANGE,                                              \
    },

struct button {
    unsigned int pio;
    bool active_high;
    unsigned int millis;
    bool raw_state;
    button_state_t state;
} buttons[] = {BUTTONS(BUTTONS_STRUCT)};

#undef BUTTONS_STRUCT

void button_init(void)
{
    if (init) {
        return;
    }

    for (unsigned int i = 0; i < ARRAY_SIZE(buttons); i++) {
        struct button *b = buttons + i;
        pio_config_set(b->pio, b->active_high ? PIO_PULLDOWN : PIO_PULLUP);
    }

    init = true;
}

bool button_update(unsigned int millis)
{
    bool changed = false;

    for (unsigned int i = 0; i < ARRAY_SIZE(buttons); i++) {
        struct button *b = buttons + i;

        if ((b->active_high == pio_input_get(b->pio)) != b->raw_state) {
            b->millis += millis;
        } else {
            b->millis = 0;
        }

        if (b->millis >= DEBOUNCE_PERIOD) {
            b->raw_state = !b->raw_state;
            b->millis = 0;
            b->state = b->raw_state ? BUTTON_DOWN : BUTTON_UP;
            changed = true;
        }
    }

    return changed;
}

button_state_t button_read(button_t button)
{
    struct button *b = buttons + button;
    const button_state_t state = b->state;
    b->state = BUTTON_NOCHANGE;
    return state;
}
