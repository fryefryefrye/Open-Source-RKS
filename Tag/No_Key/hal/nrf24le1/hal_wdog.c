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
  * @brief Implementation of hal_wdog
 */

#include <stdint.h>
#include <stdbool.h>

#include "nrf24le1.h"
#include "nordic_common.h"
#include "hal_wdog.h"

void hal_wdog_init(uint16_t start_value)
{
#ifdef __C51__
  WDSV = LSB(start_value);      // Write the 8 LSB to the WD counter
  WDSV = MSB(start_value);      // Write the 8 MSB to the WD counter
#elif __ICC8051__
  WDSVL = LSB(start_value); 
  WDSVH = MSB(start_value);
#else
#error Please define watchdog registers!
#endif
}

void hal_wdog_restart(void)
{
  uint8_t wd_msb, wd_lsb;

  #ifdef __C51__  
  wd_lsb = WDSV;
  wd_msb = WDSV;
  WDSV = wd_lsb;           // Write the 8 LSB to the WD counter
  WDSV = wd_msb;           // Write the 8 MSB to the WD counter
#elif __ICC8051__
  wd_lsb = WDSVL;
  wd_msb = WDSVH;
  WDSVL = wd_lsb;           // Write the 8 LSB to the WD counter
  WDSVH = wd_msb;           // Write the 8 MSB to the WD counter
#else
#error Please define watchdog restart
#endif
}