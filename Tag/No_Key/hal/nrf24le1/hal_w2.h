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
 * $LastChangedRevision: 2713 $
 */

/** @file
 * @brief  Interface for the 2-Wire module.
 * @defgroup hal_nrf24le1_hal_w2 2-Wire (hal_w2)
 * @{
 * @ingroup hal_nrf24le1
 *
 */

#ifndef HAL_W2_H__
#define HAL_W2_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf24le1.h"

#define W2CON0_FLAG_X_STOP      0x20
#define W2CON0_FLAG_X_START     0x10

#define W2CON1_FLAG_NACK        0x02
#define W2CON1_FLAG_DATA_READY  0x01

#define HAL_W2_ISSUE_START_COND (W2CON0 |= W2CON0_FLAG_X_START)
#define HAL_W2_ISSUE_STOP_COND (W2CON0 |= W2CON0_FLAG_X_STOP)
#define HAL_W2_WAIT_FOR_INTERRUPT {while(!SPIF); SPIF = 0; }
#define HAL_W2_WRITE(a) W2DAT = (a)
#define HAL_W2_READ() W2DAT

#if defined(NRF24LE1_48)
  #define W2_PIN_SDA P13
  #define W2_PIN_SCL P12
  #define HAL_W2_CLEAR_SDA_SCL (P1 &= 0xF3)
  #define HAL_W2_OVERRIDE_SDA_SCL(a, b) P1DIR = (P1DIR & 0xF3) | (a << 3) | (b << 2)  
#elif defined(NRF24LE1_32)
  #define W2_PIN_SDA P05
  #define W2_PIN_SCL P04
  #define HAL_W2_CLEAR_SDA_SCL (P0 &= 0xCF)
  #define HAL_W2_OVERRIDE_SDA_SCL(a, b) P0DIR = (P0DIR & 0xCF) | (a << 5) | (b << 4)
#elif defined(NRF24LE1_24)
  #define W2_PIN_SDA P06
  #define W2_PIN_SCL P05
  #define HAL_W2_CLEAR_SDA_SCL (P0 &= 0x9F)
  #define HAL_W2_OVERRIDE_SDA_SCL(a, b) P0DIR = (P0DIR & 0x9F) | (a << 6) | (b << 5)
#else
#warning "LE1 package type not defined! Add project wide definition depending on the LE1 package variant you are using. Valid options are NRF24LE1_48, NRF24LE1_32 and NRF24LE1_24."
#define W2_SOFT_RESET_NOT_AVAILABLE
#endif

/** An enum describing the clock frequency.
 *
 */
typedef enum {
  HAL_W2_IDLE   = 0x00,       /**< Idle */
  HAL_W2_100KHZ = 0x01,       /**< 100 KHz (Standard mode) */
  HAL_W2_400KHZ = 0x02        /**< 400 KHz (Fast mode) */
} hal_w2_clk_freq_t;                

/** An enum describing master or slave mode.
 *
 */
typedef enum {
  HAL_W2_MASTER,              /**< Master mode */
  HAL_W2_SLAVE                /**< Slave mode */
} hal_w2_op_mode_t;                 

/** An enum describing the 2-wire's irq sources.
 *
 */ 
typedef enum {
  HAL_W2_STOP_COND = 0x08,    /**< Interrupt caused by stop condition */
  HAL_W2_ADRESS_MATCH = 0x04, /**< Interrupt caused by address match */
  HAL_W2_DATA_READY = 0x01    /**< Interrupt caused by byte transmitted/received */
} hal_w2_irq_source_t;

/** An enum describing master transfer direction.
 *
 */
typedef enum {
  HAL_W2_DIR_WRITE,              /**< Transfer direction: write */
  HAL_W2_DIR_READ                /**< Transfer direction: read */
} hal_w2_direction_t;                 


/** @name   SLAVE SPECIFIC FUNCTIONS  */
//@{
/* Slave specific function prototypes */

/** Function to set the slave to respond to a general call address.
 * Use this function set the slave to respond to a general call address (0x00) 
 * as well as the defined address
 * -- Slave only --
 *
 * @param resp_gen True if the slave shall respond to general call adress
 */
void hal_w2_respond_to_gen_adr(bool resp_gen);

/** Function to control clockStop mode.
 * Use this function to control clockStop mode
 * -- Slave only --
 */
void hal_w2_alter_clock(bool alt_clk);

/** Function to enable irq when stop condition is detected.
 * Use this function enable irq when stop condition is detected
 * -- Slave only --
 *
 * @param stop_cond True to enable, false to disable irq when stop condition is detected
 */
void hal_w2_irq_stop_cond_enable(bool stop_cond);

/** Function to enable the irq on address match.
 * Use this function enable the irq on address match
 * -- Slave only --
 *
 * @param addr_match True to enable, false to disable the irq on address match
 */
void hal_w2_irq_adr_match_enable(bool addr_match);

/** Function to set the slave address.
 * Use this function to set the 7-bit address for slave mode.
 *
 * @param address 7-bit slave address 
 */
void hal_w2_set_slave_address(uint8_t address);
//@}

//@}

/** Function to set the clock frequency.
 * Use this function select clock frequency of the 2-wire.
 *
 * @param freq The clock frequency of the 2-wire
 */
void hal_w2_set_clk_freq(hal_w2_clk_freq_t freq);

/** Function to set the operation mode of the 2-wire.
 * Use this function select master or slave mode.
 *
 * @param mode The operation mode of the 2-wire
 */
void hal_w2_set_op_mode(hal_w2_op_mode_t mode);

/** Function to enable the 2-wire.
 * Use this function enable the 2-wire, must be done before any other
 * programming of the 2-wire.
 *
 * @param en True to enable, false to disable
 */
void hal_w2_enable(bool en);

/** Function to enable all interrupts.
 * Use this function enable all interrupts.
 *
 * @param irq True to enable, false to disable all interrupts in the 2-wire
 */
void hal_w2_all_irq_enable(bool irq);

void hal_w2_configure_master(hal_w2_clk_freq_t mode);

bool hal_w2_init_transfer(uint8_t address, hal_w2_direction_t direction);

bool hal_w2_write(uint8_t address, const uint8_t *data_ptr, uint8_t data_len);
bool hal_w2_read(uint8_t address, uint8_t *buffer, uint8_t buffer_size);

#endif // HAL_W2_H__
/** @}  */
