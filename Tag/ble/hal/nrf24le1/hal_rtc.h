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
 * @brief Interface functions for the real-time clock.
 * @defgroup hal_nrf24le1_hal_rtc Real-time clock (hal_rtc)
 * @{
 * @ingroup hal_nrf24le1
 *
 * The real-time clock is a configurable, linear, 16-bit real time clock with capture 
 * and compare capabilities. The input clock frequency is 32 KHz. This module contains 
 * functions for initializing, configuring and controlling the real-time clock.
 */

#ifndef __HAL_RTC_H__
#define __HAL_RTC_H__

#include <stdint.h>
#include <stdbool.h>

/** An enum describing the clock frequency.
 *
 */
typedef enum {
  HAL_RTC_COMPARE_MODE_DISABLE = 0x00,     /**< Compare disabled */ 
  HAL_RTC_COMPARE_MODE_1       = 0x02,     /**< Interrupt will not reset the timer */
  HAL_RTC_COMPARE_MODE_0       = 0x03      /**< The interrupt resets the timer */
} hal_rtc_compare_mode_t; 

/** Function that captures the current RCT2 value.
 */
void hal_rtc_capture(void);

/** Function that enables the timer value to be captured by an IRQ
 * from the Radio 
 *
 * @param en True to enable, false to disable capture by radio
 */
void hal_rtc_radio_capture_enable(bool en);

/** Function that sets the compare mode of RTC2.
 *
 * @param mode Compare mode
 */
void hal_rtc_set_compare_mode(hal_rtc_compare_mode_t mode);

/** Function that enables RTC2.
 *
 * @remark Remember to enable the low frequency clock before use
 * @param en True to enable, false to disable rtc
 */
void hal_rtc_start(bool en);

/** Function that sets the value to be compared to the timer value 
 * to generate interrupt.
 *
 * @param value Comparing value
 */
void hal_rtc_set_compare_value(uint16_t value);

/** Function that returns the compare value
 *
 * @return Compare value
 */
uint16_t hal_rtc_get_compare_value(void);

/** Function that returns the timer value.
 *
 * @return Timer value
 */
uint16_t hal_rtc_get_capture_value(void);

/** Function that returns the value of the counter that counts the
 * number of CPU clock cycles from the previous positive edge of the 
 * 32 KHz clock until the capture event.
 *
 * @remark External capture should be disabled when reading both capture value and clock cycles
 * @return Number of CPU clock cycles
 */
uint8_t hal_rtc_get_capture_clock_cycles(void);

#endif // __HAL_RTC_H__

/** @} */
