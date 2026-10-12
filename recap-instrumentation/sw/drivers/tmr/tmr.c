/**
 * @file tmr.c
 * @author Ben Mitchell (benjamin.mitchell@canterbury.ac.nz) and Harry Mander
 *     (harry.mander@pg.canterbury.ac.nz)
 *
 * @brief Implements timer-driven interrupts. The ATSAM4S16B has 6
 *     independently-configurable timers.
 *
 * @version 0.2
 * @date 2023-12-02
 *
 * @copyright Copyright (c) 2021, 2023
 */

#include "tmr.h"

#include <target.h>

#include <irq.h>
#include <mcu.h>

tmr_t *tmr_init(
    tmr_module_t module,
    tmr_channel_t channel,
    tmr_prescaler_t prescaler,
    uint16_t counter,
    irq_priority_t irq_pri,
    irq_vector_t isr
)
{
    tmr_t *tmr = NULL;

    if (channel <= TMR_CHANNEL_2) {
        switch (module) {
        case TMR_MODULE_0:
            tmr = TC0->TC_CHANNEL + channel;
            break;
        case TMR_MODULE_1:
            tmr = TC1->TC_CHANNEL + channel;
            break;
        default:
            break;
        }
    }

    if (tmr) {
        /**
         * Each timer channel has its own interrupt controller. There seems to
         * be an error in the datasheet in sec. 37.5.3 (p. 854), where only two
         * peripheral IDs are listed, implying that the IDs correspond to the
         * timer module (of which there are two with three channels each).
         * However, the table on p. 50 lists 6 peripheral IDs for the
         * timer/counter starting from 23 (TC0).
         */
        const uint8_t pid = ID_TC0 + module * 2 + channel;

        mcu_pmc_enable(pid);

        // configure timer mode
        tmr->TC_CMR =
            (prescaler & TC_CMR_TCCLKS_Msk) | TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC;

        // configure timer counter
        tmr->TC_RC = counter;

        // configure IRQ to fire on RC compare
        tmr->TC_IER = tmr->TC_IMR = TC_IER_CPCS;

        // register the timer handlers and enable them
        irq_config(pid, irq_pri, isr);
        irq_enable(pid);
    }

    return tmr;
}
