#pragma once

/* Status Indicators
 * Automatically handles the status LEDs to indicate various modes.
 */

typedef enum {
    STATUS_INIT,
    STATUS_IDLE,
    STATUS_RECORDING,
    STATUS_ERROR
} status_t;

/**
 * @brief Initialize the status module.
 */
void status_init(void);

/**
 * @brief Set the new mode for the status LEDs.
 *
 * @param mode INIT, IDLE, RECORDING, or ERROR.
 */
void status_set_mode(status_t mode);

/**
 * @brief Get the current status
 *
 * @returns the current mode INIT, IDLE, RECORDING, or ERROR
 */
status_t status_get_mode(void);
