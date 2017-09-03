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
@brief Interface for Power-fail comparator.
@defgroup hal_nrf24le1_hal_pof Power-fail comparator (hal_pof)
@{
@ingroup hal_nrf24le1
*
* The Power-fail comparator provides the MCU with an early warning of impending power failure.
* A flag wil be raised when the system supply voltage is below a configurable threshold voltage.
*
*/

#ifndef __HAL_POF_H__
#define __HAL_POF_H__

#include <stdint.h>
#include <stdbool.h>

/** An enum used for different POF thresholds.
 *
 */
typedef enum
{
  HAL_POF_THRESHOLD_2_1V = 0x00,
  HAL_POF_THRESHOLD_2_3V = 0x20,
  HAL_POF_THRESHOLD_2_5V = 0x40,
  HAL_POF_THRESHOLD_2_7V = 0x60
} hal_pof_threshold_t;

/** Function to enable or disable the Power-fail comparator
 *
 * @param enable Boolean value. True = enable, false = disable.
 */
void hal_pof_enable(bool enable);

/** Function setting the threshold voltage for the Power-fail comparator
 *
 * @param threshold selects the threshold for the Power-fail comparator
 * @arg HAL_POF_THRESHOLD_2_1V
 * @arg HAL_POF_THRESHOLD_2_3V
 * @arg HAL_POF_THRESHOLD_2_5V
 * @arg HAL_POF_THRESHOLD_2_7V
 */
void hal_pof_set_threshold(hal_pof_threshold_t threshold);

/**
 * Checks for low voltage warning from the Power-fail comparator
*/
bool hal_pof_warning(void);


#endif // __HAL_POF_H__

/** @} */
