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
 * $LastChangedRevision: 133 $
 */

/** @file
 * @brief Implementation of hal_rng
 */

#include <stdint.h>
#include <stdbool.h>

#include "nrf24le1.h"
#include "nordic_common.h"
#include "hal_rng.h"

void hal_rng_power_up(bool pwr_up)
{
  if(pwr_up)
  {
    RNGCTL = (RNGCTL | BIT_7);      // powerUp bit = 1, power up the RNG
  }
  else
  {
    RNGCTL = (RNGCTL & ~(BIT_7));   // powerUp bit = 0, turn off the RNG
  }
}

void hal_rng_bias_corr_enable(bool en)
{
  if(en)
  {
    RNGCTL = (RNGCTL | BIT_6);      // correctorEn bit = 1, turn on corrector
  }
  else
  {
    RNGCTL = (RNGCTL & ~(BIT_6));   // correctorEn bit = 0, turn off corrector
  }
}

uint8_t hal_rng_read(void)
{
  return RNGDAT;                    // Return data stored in RNGDAT register
}

bool hal_rng_data_ready(void)
{
  return((RNGCTL & BIT_5) > 0);     // Return true if data ready bit is 1
}
