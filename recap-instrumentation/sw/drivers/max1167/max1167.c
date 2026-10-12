/**
 * @file max1167.c
 * @author Ben Mitchell (benjamin.mitchell@canterbury.ac.nz)
 * @brief Device driver for the MAX1167 SPI ADC. This is a fairly barebones
 * driver that allows sampling of the four analog channels and voltage
 * conversion.
 * @version 0.1
 * @date 2021-12-02
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "max1167.h"

#include "target.h"

#include "delay.h"

static spi_t spi;

#define CHANNEL_SEL(ch) ((ch) << 5)
#define EXTERNAL_REF (BIT(2) | BIT(1))

/**
 * @brief Initialize the MAX1167 SPI ADC module. Really this just initializes
 * the SPI communications and then warms up the voltage reference.
 *
 * @param cfg Configuration of the SPI bus to use.
 * @return true if the device is ready to use.
 * @return false if the device is not ready.
 */
bool max1167_init(const spi_cfg_t *cfg)
{
    spi = spi_init(cfg);

    spi_mode_set(spi, SPI_MODE_0);
    spi_cs_mode_set(spi, SPI_CS_MODE_FRAME);

    // let the voltage reference warm up
    spi_cs_assert(spi);
    delay_ms(10);

    // there isn't a good way to check if the device is working correctly, but
    // since it is soldered on, it is a reasonably safe assumption.
    return true;
}

/**
 * @brief Read from a single chanel on the ADC.
 *
 * @param channel The ADC channel to read [0-3].
 * @return uint16_t The raw sampled value.
 */
uint16_t max1167_read(uint8_t channel)
{
    // Single acquisition from `channel`, external reference, external clock
    uint8_t data[3] = {
        CHANNEL_SEL(channel) | EXTERNAL_REF,
        0,
        0,
    };

    spi_transfer(spi, data, data, sizeof(data), true);

    uint16_t raw = ((uint16_t) data[1] << 8) | data[2];
    return *(int16_t *) &raw;
}
