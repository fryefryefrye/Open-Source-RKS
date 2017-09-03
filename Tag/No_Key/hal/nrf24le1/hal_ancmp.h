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
 * $LastChangedRevision: 162 $
 */ 

/** @file
 * @brief Interface functions for the analog comparator. 
 * @defgroup hal_nrf24le1_hal_ancmp Analog comparator (hal_ancmp)
 * @{
 * @ingroup hal_nrf24le1
 *
 * The analog comparator allows a system wakeup to be triggered by the voltage 
 * level of a differential or single-ended analog input applied through the port pins.
 *
 * This module also contains functions for configuring and enabling the comparator.
 */

#ifndef HAL_ANCMP_H__
#define HAL_ANCMP_H__

#include <stdint.h>
#include <stdbool.h>
#include "hal_adc.h"

/** An enum describing the input channel of the analog comparator.
 *
 */
typedef enum {
  HAL_ANCMP_INP_AIN0   = HAL_INP_AIN0,
  HAL_ANCMP_INP_AIN1   = HAL_INP_AIN1,
  HAL_ANCMP_INP_AIN2   = HAL_INP_AIN2,
  HAL_ANCMP_INP_AIN3   = HAL_INP_AIN3,
  HAL_ANCMP_INP_AIN4   = HAL_INP_AIN4,
  HAL_ANCMP_INP_AIN5   = HAL_INP_AIN5,
  HAL_ANCMP_INP_AIN6   = HAL_INP_AIN6,
  HAL_ANCMP_INP_AIN7   = HAL_INP_AIN7,
  HAL_ANCMP_INP_AIN8   = HAL_INP_AIN8,
  HAL_ANCMP_INP_AIN9   = HAL_INP_AIN9, 
  HAL_ANCMP_INP_AIN10  = HAL_INP_AIN10,
  HAL_ANCMP_INP_AIN11  = HAL_INP_AIN11,
  HAL_ANCMP_INP_AIN12  = HAL_INP_AIN12,
  HAL_ANCMP_INP_AIN13  = HAL_INP_AIN13
} hal_ancmp_input_channel_t;

/** An enum describing the reference of the analog comparator.
 *
 */
typedef enum {
  HAL_ANCMP_NON_INVERTING = 0x00,  /**< Non-inverting output polarity */
  HAL_ANCMP_INVERTING     = 0x01   /**< Inverting output polarity */
} hal_ancmp_pol_t;                  

/** An enum describing the reference of the analog comparator.
 *
 */
typedef enum {
  HAL_ANCMP_REF_SCALE_25  = 0x00,      /**< Reference voltage scaling (25%) */
  HAL_ANCMP_REF_SCALE_50  = 0x01,      /**< Reference voltage scaling (50%) */
  HAL_ANCMP_REF_SCALE_75  = 0x02,      /**< Reference voltage scaling (75%) */
  HAL_ANCMP_REF_SCALE_100 = 0x03,      /**< Reference voltage scaling (100%) */
} hal_ancmp_ref_scale_t;

/** An enum describing the reference of the analog comparator.
 *
 */
typedef enum {
  HAL_ANCMP_REF_VDD = 0x00,            /**< VDD as reference */
  HAL_ANCMP_REF_EXT = 0x01             /**< External reference */
} hal_ancmp_ref_t;


/** Function to set the output polarity.
 * Use this function set the output polarity of the analog comparator.
 *
 * @param polarity The output polarity
 */
void hal_ancmp_set_polarity(hal_ancmp_pol_t polarity);

/** Function to set the reference voltage scaling.
 * Use this function to set reference voltage scaling of the analog comparator.
 *
 * @param scale The reference voltage scaling
 */
void hal_ancmp_set_ref_voltage_scale(hal_ancmp_ref_scale_t scale);

/** Function to set the reference of the analog comparator.
 * Use this function to set the reference of the analog comparator.
 *
 * @param ref The reference of the analog comparator
 */
void hal_ancmp_set_reference(hal_ancmp_ref_t ref);

/** Function to set the input channel of the analog comparator.
 * This function uses the ADC HAL to set the input channel of the analog comp.
 *
 * @param chsel The input channel of the analog comparator
 */
void hal_ancmp_set_input_channel(hal_ancmp_input_channel_t chsel);

/** Function to enable the analog comparator.
 * Use this function to enable the analog comparator.
 *
 * @param en True to enable, false to disable the analog comparator
 */
void hal_ancmp_enable(bool en);


#endif // HAL_ANCMP_H__
/** @}  */

