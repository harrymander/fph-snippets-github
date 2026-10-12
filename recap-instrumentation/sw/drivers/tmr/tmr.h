/**
 * @file tmr.h
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

#pragma once

#include <irq.h>
#include <mcu.h>

#include <stdint.h>

typedef TcChannel tmr_t;

typedef enum {
    TMR_MODULE_0 = 0,
    TMR_MODULE_1,
} tmr_module_t;

typedef enum {
    TMR_CHANNEL_0 = 0,
    TMR_CHANNEL_1,
    TMR_CHANNEL_2,
} tmr_channel_t;

typedef enum {
    TMR_PRESCALER_2 = 0,
    TMR_PRESCALER_8,
    TMR_PRESCALER_32,
    TMR_PRESCALER_128
} tmr_prescaler_t;

/**
 * @brief Configure the timing for a given timer channel.
 *
 * @param module Timer module.
 * @param channel Timer channel.
 * @param prescaler Prescaler for the particular channel [2, 8, 32, 128].
 * @param counter Timer period in F_CPU / Prescaler ticks.
 * @param irq_pri Timer interrupt priority.
 * @param isr Interrupt service routine.
 *
 * @return Pointer to timer handle, or NULL on error.
 */
tmr_t *tmr_init(
    tmr_module_t module,
    tmr_channel_t channel,
    tmr_prescaler_t prescaler,
    uint16_t counter,
    irq_priority_t irq_pri,
    irq_vector_t isr
);

/**
 * @brief Start the timer channel.
 *
 * @param tmr Timer handle; undefined behaviour if NULL.
 */
static inline void tmr_start(tmr_t *tmr)
{
    tmr->TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
}

/**
 * @brief Stop the timer channel.
 *
 * @param tmr Timer handle; undefined behaviour if NULL.
 */
static inline void tmr_stop(tmr_t *tmr)
{
    tmr->TC_CCR = TC_CCR_CLKDIS;
}

/**
 * @brief Clear the timer interrupt.
 * @param tmr Timer handle; undefined behaviour if NULL;
 * @return true if the interrupt triggered since it was last cleared.
 */
static inline bool tmr_clear_irq(const tmr_t *tmr)
{
    // Reading the status register clears the interrupt
    return tmr->TC_SR & TC_SR_CPCS;
}
