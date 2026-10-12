#pragma once

#include <irq.h>
#include <twi-async.h>

#include <inttypes.h>
#include <stdint.h>

typedef int16_t sdp810_pressure_t;

/**
 * @brief Initialise the SDP810 sensor on the I2C bus. Sends the start
 * continuous measurement command allowing immediate get flow commands.
 *
 * @param twi TWI (I2C) async interface to communicate over.
 * @return true if the device was correctly initialised.
 * @return false if the SDP810 did not respond to the initialize command.
 */
bool sdp810_init(twi_async_t *twi);

/**
 * Begins an asynchronous read.
 *
 * @return 0 on success, or a negative integer on error
 */
int sdp810_async_read(void);

/**
 * Polls the asynchronous read. Must have called `sdp810_async_read` first.
 *
 * Do not call again after asynchronous read has completed before calling
 * `sdp810_async_read`.
 *
 * @param pressure Pointer to variable in which to store the raw pressure
 *     measurement. This can be converted into pascals using the formula:
 *
 *       pressure[pascals] = pressure[raw] / 60.0
 *
 * @return Positive integer if read complete, 0 if not finished, negative
 *     integer on error. Value of `*pressure` unchanged if return value not
 *     positive.
 */
int sdp810_async_poll(sdp810_pressure_t *pressure);
