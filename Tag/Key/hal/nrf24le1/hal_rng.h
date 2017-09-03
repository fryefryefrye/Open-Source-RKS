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
 * $LastChangedRevision: 165 $
 */

/** @file
 * @brief Interface functions for the true random number generator.
 * @defgroup hal_nrf24le1_hal_rng Random number generator (hal_rng)
 * @{
 * @ingroup hal_nrf24le1
 *
 *
 * The Random Number Generator (RNG) uses thermal noise to produce a non-deterministic 
 * bit stream. A digital corrector algorithm is used on the bit stream to remove 
 * any bias. The bits are then queued into an 8-bit register for parallel readout.
 *
 * This module contains functions for initializing and enabling the number generator, 
 * for checking the status of the generator and for reading one random number.
 */


#ifndef HAL_RNG_H__
#define HAL_RNG_H__

#include <stdint.h>
#include <stdbool.h>

/** Function to power up and power down the random number generator.
 * When power up is set, the random number generator starts
 * generating a random number.
 *
 * @param pwr_up True to power up, false to power down
 */
void hal_rng_power_up(bool pwr_up);

/** Function to control the bias corrector on the random number generator.
 * Use this function to enable or disable the bias corrector.
 *
 * @param en True to enable, false to disable bias corrector
 */
void hal_rng_bias_corr_enable(bool en);

/** Function to read the random data register on the random number generator.
 * Use this function to get the random number from the random number generator.
 * 
 * @return Random data
 */
uint8_t hal_rng_read(void);

/** Function that returns the status of the RNG.
 * This function returns true if there is data ready from the RNG.
 *
 * @return RNG Data ready bit
 * @retval FALSE no data available
 * @retval TRUE data available
 */
bool hal_rng_data_ready(void);

#endif // HAL_RNG_H__
/** @}  */

