/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic 
 * Semiconductor ASA.Terms and conditions of usage are described in detail 
 * in NORDIC SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT. 
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRENTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *              
 * $LastChangedRevision: 168 $
 */

/** @file
 * @brief Interface functions for the Universal Asynchronous Receiver-Transmitter (UART).
 *
 * @defgroup hal_nrf24le1_hal_uart Universal Asynchronous Receiver-Transmitter (hal_uart)
 * @{
 * @ingroup hal_nrf24le1
 *
 * The MCU system is configured with one serial port that is identical in operation 
 * to the standard 8051 serial port (Serial interface 0). The two serial port signals 
 * RXD and TXD are available on device pins UART/RSD and UART/TXD. The serial port (UART) 
 * derives its clock from the MCU clock
 *
 * This module contains functions for configuring the baud rate of the UART and 
 * for "getting" and "putting" character from/to the serial interface. Circular 
 * buffering is added in firmware to ensure a user friendly and reliable communication. 
 *
 */
 
#ifndef HAL_UART_H__
#define HAL_UART_H__

#include <stdint.h>
#include <stdbool.h>

/** Available Baud rates.
 * The input argument of hal_uart_init must be defined in this @c enum
 */
typedef enum
{
    UART_BAUD_600,      ///< 600 baud
    UART_BAUD_1K2,      ///< 1200 baud
    UART_BAUD_2K4,      ///< 2400 baud
    UART_BAUD_4K8,      ///< 4800 baud
    UART_BAUD_9K6,      ///< 9600 baud
    UART_BAUD_19K2,     ///< 19.2 kbaud
    UART_BAUD_38K4,     ///< 38.4 kbaud
    UART_BAUD_57K6,     ///< 57.6 kbaud
    UART_BAUD_115K2     ///< 115.2 kbaud
} hal_uart_baudrate_t;

/** Function to initialize the UART.
 * This function initializes the UART for interrupt operation. An 8 byte receive
 * buffer and an 8 byte transmit buffer is used.
 * @param baud a constant for the baud rate (0 = 600 Baud, ..., 7=57600 baud)
 */
void hal_uart_init(hal_uart_baudrate_t baud);

/** Function to write a character to the UART transmit buffer.
 * @param ch Character to write
 */
void hal_uart_putchar(uint8_t ch);

/** Function for checking entire TX FIFO is empty.
 * @return true if all data sent, else false.
 */
bool hal_uart_tx_complete();

/** @brief Find number of characters in the UART receive buffer
 *
 * This function returns the number of characters available for reading
 * in the UART receive buffer.
 * 
 * @return Number of characters available
 */
uint8_t hal_uart_chars_available(void);

/** Function to read a character from the UART receive buffer.
 * @return Character read
 */
uint8_t hal_uart_getchar(void);

#endif // HAL_UART_H__
/** @} */
