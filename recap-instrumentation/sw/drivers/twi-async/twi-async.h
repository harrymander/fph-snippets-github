/**
 * Piggy-backs off the twi module in mat91lib to provide an asynchronous
 * interrupt-based master read interface.
 *
 * Adapted from Michael P. Hayes' TWI driver in mat91lib.
 *
 * Copyright (c) Harry Mander 2023
 */

#ifndef RECAP_DRIVERS_TWI_ASYNC_H
#define RECAP_DRIVERS_TWI_ASYNC_H

#include <irq.h>
#include <twi.h>

typedef struct twi_async_read_fut twi_async_read_fut_t;
typedef void twi_async_t;

typedef enum {
    TWI_ASYNC_ERROR = -1,
    TWI_ASYNC_PENDING,
    TWI_ASYNC_SUCCESS,
} twi_async_ret_t;

/**
 * Initialise the TWI module for asynchronous operation.
 *
 * @param cfg TWI configuration.
 * @param priority TWI interrupt priority
 * @return Pointer to twi async handle, NULL on error.
 */
twi_async_t *twi_async_init(const twi_cfg_t *cfg, irq_priority_t priority);

/**
 * Begins a master read transaction without internal address.
 *
 * Returns a 'future' object, which must be polled with `twi_async_poll`. Do not
 * call other twi functions on the twi object until the future has completed.
 *
 * @param twi A twi handle as instantiated by `twi_init`.
 * @param slave_addr The slave address to read from.
 * @param buf Pointer to buffer to store read data into.
 * @param len Number of bytes to read.
 * @return Pointer to future object, NULL on error.
 */
twi_async_read_fut_t *twi_async_begin_read(
    twi_async_t *twi, uint8_t slave_addr, volatile uint8_t *buf, twi_size_t size
);

/**
 * Poll the TWI read operation.
 *
 * @param fut Read future as returned by `twi_async_begin_read`.
 * @return The status of the read operation; as follows:
 *   - TWI_ASYNC_SUCCESS: Read operation completed successfully.
 *   - TWI_ASYNC_PENDING: Read operation still pending, call again later.
 *   - TWI_ASYNC_ERROR: Error, further calls to this function are UB.
 */
twi_async_ret_t twi_async_poll(const twi_async_read_fut_t *fut);

#endif // RECAP_DRIVERS_TWI_ASYNC_H
