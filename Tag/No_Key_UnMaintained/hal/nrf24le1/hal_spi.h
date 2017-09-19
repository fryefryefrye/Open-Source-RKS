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
 * $LastChangedRevision: 167 $
 */

/** @file
 * @brief Interface functions for the Serial Peripheral Interface (SPI).
 *
 * @defgroup hal_nrf24le1_hal_spi Serial Peripheral Interface (hal_spi)
 * @{
 * @ingroup hal_nrf24le1
 * 
 * The Serial Peripheral Interface (SPI) is double buffered and can be configured to 
 * work in all four SPI modes. The default is mode 0.
 *
 * The SPI connects to the following pins of the device: MMISO, MMOSI, MSCK.
 * The SPI Master function does not generate any chip 
 * select signal (CSN). The programmer typically uses another programmable 
 * digital I/O to act as chip selects for one or more external SPI Slave devices.
 * 
 * This module contains functions for initializing the SPI master; reading and 
 * writing single and multiple bytes from/to the SPI master; and flushing the SPI RX FIFO.
 */
#ifndef HAL_SPI_H__
#define HAL_SPI_H__

#include <stdint.h>
#include <stdbool.h>

/** Available SPI clock divide factors
 * The input argument of hal_spi_master_init must be defined in this @c enum
 */
typedef enum
{
    SPI_CLK_DIV2,        /**< CPU clock/2 */
    SPI_CLK_DIV4,        /**< CPU clock/4 */
    SPI_CLK_DIV8,        /**< CPU clock/8 */
    SPI_CLK_DIV16,       /**< CPU clock/16 */
    SPI_CLK_DIV32,       /**< CPU clock/32 */
    SPI_CLK_DIV64,       /**< CPU clock/64 */
    SPI_CLK_DIV128       /**< CPU clock/128 */
} hal_spi_clkdivider_t;  

/** SPI data order (bit wise per byte) on serial output and input
 * The input argument of hal_spi_master_init must be defined in this @c enum
 */
typedef enum {
   HAL_SPI_LSB_MSB,      /**< LSBit first, MSBit last */
   HAL_SPI_MSB_LSB       /**< MSBit first, LSBit last */
} hal_spi_byte_order_t;
    
/** SPI operating modes.
 * The input argument of hal_spi_master_init must be defined in this @c enum
 */
typedef enum {           
   HAL_SPI_MODE_0,       /**< MSCK/SSCK is active high, sample on leading edge of MSCK/SSCK */
   HAL_SPI_MODE_1,       /**< MSCK/SSCK is active high, sample on trailing edge of MSCK/SSCK */
   HAL_SPI_MODE_2,       /**< MSCK/SSCK is active low, sample on leading edge of MSCK/SSCK */
   HAL_SPI_MODE_3        /**< MSCK/SSCK is active low, sample on trailing edge of MSCK/SSCK */
} hal_spi_mode_t;


/** Function to initialize and enable the SPI master.
 */
void hal_spi_master_init(hal_spi_clkdivider_t ck, hal_spi_mode_t mode, hal_spi_byte_order_t bo);

/** Function to read/write a byte on the SPI master.
 * This function writes a byte to the SPI master, then enters an infinite loop
 * waiting for the operation to finish. When the operation is finished the SPI
 * master interrupt bit is cleared and the received byte is returned
 * @param pLoad byte to write
 * @return The read byte
 */
uint8_t hal_spi_master_read_write(uint8_t pLoad);

/** Function to initialize and enable the SPI slave.
 */
void hal_spi_slave_init(hal_spi_mode_t mode, hal_spi_byte_order_t bo);

/** Function to read/write a byte on the SPI slave.
 * This function enters an infinite loop waiting for the data to be ready.
 * Then the data is saved and the byte is written to the SPI slave.
 * When the operation is finished the received byte is returned.
 * @param pLoad byte to write
 * @return The read byte
 */
uint8_t hal_spi_slave_rw(uint8_t pLoad);

/** Function to read the data register of SPI slave 
 * @return The read byte                           .
 */
uint8_t hal_spi_slave_read(void);

/** Function to write data to the SPI slave. 
 * @param pLoad byte to write
 */
void hal_spi_slave_preload(uint8_t pLoad);

/** Function to check if data is available in RX FIFO of SPI slave.
 * @return Status of rxDataReady bit
 */
bool spi_slave_data_ready(void);

bool hal_spi_slave_csn_high(void);


#endif // HAL_SPI_H__
/** @} */

