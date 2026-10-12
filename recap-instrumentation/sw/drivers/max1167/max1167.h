#pragma once
#include <spi.h>

#include <stdint.h>

/**
 * @brief Initialise the MAX1167 sensor on the SPI bus.
 *
 * @param cfg SPI connection configuration
 * @return true if the device was correctly initialised.
 * @return false if the MAX1167 is not working.
 */
bool max1167_init(const spi_cfg_t *cfg);

/**
 * @brief Gets the current ADC measurement.
 *
 * @return int16_t The 16 bit pressure measurement. This must be decoded later.
 */
uint16_t max1167_read(uint8_t channel);
