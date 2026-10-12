/* RECAP Data Collection Board v2 */

#ifndef TARGET_H
#define TARGET_H

#include "mat91lib.h"

#define RECAP_BOARD_NAME "v2"

/* System clocks  */
#define F_XTAL 12.00e6
#define MCU_PLL_MUL 20
#define MCU_PLL_DIV 1

#define MCU_USB_DIV 2
/* 240 MHz  */
#define F_PLL (F_XTAL / MCU_PLL_DIV * MCU_PLL_MUL)
/* 120 MHz  */
#define F_CPU (F_PLL / 2)

/* Sensors */
#define SDP810_ADDR 0x25
#define SDP810_TWI TWI_CHANNEL_1
#define SFM3XXX_ADDR 0x40
#define SFM3XXX_DEV_3000_200
#define SFM3XXX_TWI TWI_CHANNEL_0

/* Microphone ADC channels */
#define MASK_MIC_ADC_CH 0
#define EAR_MIC_ADC_CH 1

/* SD Card */
#define SD_MOSI PA22_PIO
#define SD_MOSI_PERIPH PIO_PERIPH_A
#define SD_MISO PA21_PIO
#define SD_MISO_PERIPH PIO_PERIPH_A
#define SD_SCLK PA23_PIO
#define SD_SCLK_PERIPH PIO_PERIPH_A
#define SD_NSS PA24_PIO

#define SD_DETECT_PIO PA15_PIO
#define SD_DETECT_PIO_CONFIG PIO_PULLUP
#define SD_DETECT_ACTIVE_HIGH 0

/* USB  */
// #define USB_VBUS_PIO PA5_PIO
#define USB_CURRENT_MA 500
#define USB_CDC_TX_RING_SIZE 1024

/* LEDs  */
#define LED1_PIO PA0_PIO
#define LED2_PIO PA1_PIO
#define LED_RED_PIO LED1_PIO
#define LED_BLU_PIO LED2_PIO
#define LED_ERROR_PIO LED_RED_PIO
#define LED_STATUS_PIO LED_BLU_PIO
#define LED_ACTIVE_HIGH false

/* Buttons  */
#define BTN1_PIO PA7_PIO
#define BTN2_PIO PA8_PIO
#define BTN3_PIO PA9_PIO

#define RECORD_BTN_PIO BTN1_PIO
#define EVENT_BTN_PIO BTN2_PIO

#define TIMING_TEST_PIO BTN3_PIO

#endif /* TARGET_H  */
