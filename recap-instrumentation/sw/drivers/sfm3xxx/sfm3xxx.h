/**
 * @file sfm3xxx.c
 * @author Ben Mitchell (benjamin.mitchell@canterbury.ac.nz), Harry Mander
 *     (harry.mander@pg.canterbury.ac.nz)
 * @brief Device driver for the Sensiron SFM3xxx range of flow sensors. Handles
 *     device initialization, data collection, and conversion into useful
 *     values.
 * @version 0.2
 * @date 2023-11-06
 *
 * Must define SFM3XXX_ADDR in target.h to the I2C address of the device, and
 * define one of:
 *
 * - SFM3XXX_DEV_3000_200
 * - SFM3XXX_DEV_3003_300_CE_AIR
 *
 * @copyright Copyright (c) 2023
 */

#pragma once

#include <irq.h>
#include <twi-async.h>

#include <inttypes.h>
#include <stdint.h>

// clang-format off
#if defined(SFM3XXX_DEV_3000_200)
typedef uint16_t sfm3xxx_flow_t;
#define SFM3XXX_DEV_NAME "SFM3000-200"
#define SFM3XXX_HEADER_SIGNED "0"
#define SFM3XXX_HEADER_SCALE "140"
#define SFM3XXX_HEADER_OFFSET "32000"
#elif defined(SFM3XXX_DEV_3003_300_CE_AIR)
typedef int16_t sfm3xxx_flow_t;
#define SFM3XXX_DEV_NAME "SFM3003-300-CE (air)"
#define SFM3XXX_HEADER_SIGNED "1"
#define SFM3XXX_HEADER_SCALE "120"
#define SFM3XXX_HEADER_OFFSET "-12288"
#else
#error SFM3XXX device not defined
#endif
// clang-format on

/**
 * @brief Initialise the SFM3xxx sensor on the I2C bus. Sends the start
 * continuous measurement command allowing immediate get flow commands.
 *
 * @param twi TWI (I2C) async interface to communicate over.
 * @return true if the device was correctly initialised.
 * @return false if the device did not respond to the initialize command.
 */
bool sfm3xxx_init(twi_async_t *twi);

/**
 * Begins an asynchronous read.
 *
 * @return 0 on success, or a negative integer on error
 */
int sfm3xxx_async_read(void);

/**
 * Polls the asynchronous read. Must have called `sfm3xxx_async_read` first.
 *
 * Do not call again after asynchronous read has completed before calling
 * `sfm3xxx_async_read`.
 *
 * @param flow Pointer to variable in which to store the raw flow measurement.
 *     (Conversion to real-world units depends on the specific device model
 *     used.)
 * @return Positive integer if read complete, 0 if not finished, negative
 *     integer on error. Value of `*flow` unchanged if return value not
 *     positive.
 */
int sfm3xxx_async_poll(sfm3xxx_flow_t *flow);
