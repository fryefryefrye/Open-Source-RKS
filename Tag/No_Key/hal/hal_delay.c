/* Copyright (c) 2009 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRENTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $LastChangedRevision: 133 $
 */

/** @file
 * @brief delay routine which is more or less exact within + - 2us.
 * @defgroup hal_nrf24le1_hal_delay Delay routine
 * @{
 * @ingroup hal_nrf24le1
 *
 *
 * Delay routine which accesses registers directly.
 *
*/

#include "hal_delay.h"
#include "memdefs.h"
//#include <stdint.h>
//#include "assertions.h"

#ifdef __C51__
#include <intrins.h>
#elif __ICC8051__
#include <intrinsics.h>
#warning Not tested properly for IAR.
#endif

void delay_us(uint16_t us)
{
  do
  {
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
  } while (--us);
}

void delay_ms(uint16_t ms)
{
  do
  {
    delay_us(250);
    delay_us(250);
    delay_us(250);
    delay_us(250);
  } while (--ms);
}

/** @} */
