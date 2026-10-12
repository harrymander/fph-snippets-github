/**
 * @file sdp810.c
 * @author Ben Mitchell (benjamin.canterbury.ac.nz)
 * @brief Device driver for the SDP810 Sensiron pressure I2C sensor. Initializes
 * a pressure sensor on the given I2C bus and allows readings from it.
 * @version 0.1
 * @date 2021-12-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "sdp810.h"

#include "target.h"

#include <delay.h>
#include <twi-async.h>

static twi_async_t *twi;
static twi_async_read_fut_t *future;
static volatile uint8_t read_buf[3];

static bool wait_for_sample(int timeout)
{
    if (sdp810_async_read() == 0) {
        sdp810_pressure_t val;
        delay_ms(timeout);
        return sdp810_async_poll(&val) > 0;
    }

    return false;
}

/**
 * @brief Initialize the pressure sensor.
 *
 * @param interface I2C interface the device is connected to.
 * @return true if the device is connected and responding.
 * @return false if the device is not connected.
 */
bool sdp810_init(twi_async_t *interface)
{
    twi = interface;

    // start continuous measurement
    uint8_t start[] = {0x36, 0x08};
    if (twi_master_write(twi, SDP810_ADDR, start, sizeof(start))
        == sizeof(start))
    {
        delay_ms(25);
        return wait_for_sample(10);
    }
    return false;
}

static inline void convert_buf(sdp810_pressure_t *pressure)
{
    // have to swap the bytes around
    const uint16_t raw = ((uint16_t) read_buf[0] << 8) | read_buf[1];
    *pressure = *(sdp810_pressure_t *) &raw;
}

int sdp810_async_read(void)
{
    future = twi_async_begin_read(twi, SDP810_ADDR, read_buf, sizeof(read_buf));
    return future ? 0 : -1;
}

int sdp810_async_poll(sdp810_pressure_t *pressure)
{
    switch (twi_async_poll(future)) {
    case TWI_ASYNC_ERROR:
        return -1;
    case TWI_ASYNC_SUCCESS:
        convert_buf(pressure);
        future = NULL;
        return 1;
    case TWI_ASYNC_PENDING:
        break;
    }

    return 0;
}
