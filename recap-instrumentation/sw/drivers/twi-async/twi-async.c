#include "twi-async.h"

#include <bits.h>
#include <irq.h>

#define IRQ_MASK                                                               \
    (TWI_IER_TXCOMP | TWI_IER_RXRDY | TWI_IER_NACK | TWI_IER_ARBLST            \
     | TWI_IER_OVRE)

struct twi_async_read_fut {
    twi_t twi;
    volatile uint8_t *buf;
    twi_size_t size;
    volatile twi_size_t read;

    volatile enum {
        STATE_READING,
        STATE_TXCOMP,
        STATE_FINISHED,
        STATE_ERROR,
    } state;
};

#define NUM_TWI_DEVICES 2
static struct twi_async_read_fut futures[NUM_TWI_DEVICES];

// From twi.c
static void master_init(twi_t twi, twi_slave_addr_t slave_addr)
{
    /* Switch to master mode.  */
    twi->base->TWI_CR = TWI_CR_MSDIS;
    twi->base->TWI_CR = TWI_CR_SVDIS; // cppcheck-suppress redundantAssignment
    twi->base->TWI_CR = TWI_CR_MSEN;  // cppcheck-suppress redundantAssignment
    twi->mode = TWI_MODE_MASTER;

    /* The flowchart Fig 33-17 suggests this order for MMR and IADR.  */
    twi->base->TWI_MMR =
        TWI_MMR_DADR(slave_addr) | BITS(0, 8, 9) | TWI_MMR_MREAD;
    twi->base->TWI_IADR = 0;
}

static void irq_handler(twi_async_read_fut_t *fut)
{
    Twi *base = fut->twi->base;
    const uint32_t status = base->TWI_SR;

    if (fut->state == STATE_READING) {
        if (status & (TWI_SR_NACK | TWI_SR_ARBLST | TWI_SR_OVRE)) {
            fut->state = STATE_ERROR;
        } else if (status & TWI_SR_RXRDY) {
            fut->buf[fut->read] = base->TWI_RHR;
            fut->read += 1;
            if (fut->read == fut->size) {
                fut->state = STATE_TXCOMP;
            } else if ((fut->read == fut->size - 1) && (fut->size > 1)) {
                // Do not ack the last byte
                base->TWI_CR = TWI_CR_STOP;
            }
        }
    } else if ((fut->state == STATE_TXCOMP) && (status & TWI_SR_TXCOMP)) {
        // Disable interrupt
        base->TWI_IDR = IRQ_MASK;
        base->TWI_SR; // cppcheck-suppress constStatement
        fut->state = STATE_FINISHED;
    }
}

static void ch0_irq_handler(void)
{
    irq_handler(&futures[0]);
}

static void ch1_irq_handler(void)
{
    irq_handler(&futures[1]);
}

twi_async_t *twi_async_init(const twi_cfg_t *cfg, irq_priority_t priority)
{
    irq_config(
        ID_TWI0 + cfg->channel,
        priority,
        cfg->channel == TWI_CHANNEL_0 ? ch0_irq_handler : ch1_irq_handler
    );
    return twi_init(cfg);
}

twi_async_read_fut_t *twi_async_begin_read(
    twi_async_t *twi_async,
    uint8_t slave_addr,
    volatile uint8_t *buf,
    twi_size_t size
)
{
    twi_t twi = twi_async;
    twi_async_read_fut_t *fut = futures + twi->channel;
    fut->twi = twi;
    fut->buf = buf;
    fut->size = size;
    fut->read = 0;
    fut->state = STATE_READING;

    fut->twi->base->TWI_IER = IRQ_MASK;
    master_init(twi, slave_addr);
    if (size == 1) {
        twi->base->TWI_CR = TWI_CR_START | TWI_CR_STOP;
    } else {
        twi->base->TWI_CR = TWI_CR_START;
    }

    irq_enable(ID_TWI0 + twi->channel);
    return fut;
}

twi_async_ret_t twi_async_poll(const twi_async_read_fut_t *fut)
{
    if (fut->state == STATE_FINISHED) {
        return TWI_ASYNC_SUCCESS;
    } else if (fut->state == STATE_ERROR) {
        return TWI_ASYNC_ERROR;
    }

    return TWI_ASYNC_PENDING;
}
