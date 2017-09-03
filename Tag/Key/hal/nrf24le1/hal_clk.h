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
 * $LastChangedRevision: 2513 $
 */

/** @file
 * @brief Interface for clock management.
 * @defgroup hal_nrf24le1_hal_clk Clock control (hal_clk)
 * @{
 * @ingroup hal_nrf24le1
 *
 * The clock to the MCU is sourced from either an on-chip RC oscillator
 * or an external crystal oscillator. This module contains functions for selecting
 * clock source and clock frequency.
 */

#ifndef HAL_CLK_H__
#define HAL_CLK_H__

#include <stdint.h>
#include <stdbool.h>

/** An enum describing the possible system clock inputs.
 *
 */
typedef enum
{
  HAL_CLK_XOSC16_OR_RCOSC16 = 0,
  HAL_CLK_PAD_XC1 = 1
} hal_clk_input_t;

/** An enum used for selecting which 16MHz clocks to run.
 *
 */
typedef enum
{
  HAL_CLK_XOSC16M_AND_RCOSC16M = 0,
  HAL_CLK_RCOSC16M = 1,
  HAL_CLK_XOSC16M = 2
} hal_clk_source_t;

/** An enum describing the possible internal (system) clock frequencies.
 *
 */
typedef enum
{
  HAL_CLK_16MHZ = 0,
  HAL_CLK_8MHZ = 1,
  HAL_CLK_4MHZ = 2,
  HAL_CLK_2MHZ = 3,
  HAL_CLK_1MHZ = 4,
  HAL_CLK_500KHZ = 5,
  HAL_CLK_250KHZ = 6,
  HAL_CLK_125KHZ = 7
} hal_clk_freq_t;

/** An enum describing the possible 32khz clock (clklf) sources.
 *
 */
typedef enum
{
  HAL_CLKLF_XOSC32K = 0,
  HAL_CLKLF_RCOSC32K = 1,
  HAL_CLKLF_XOSC16M_SYNTH = 2,
  HAL_CLKLF_PAD_XC1 = 3,
  HAL_CLKLF_DIG_IO_RAIL = 4,
  HAL_CLKLF_NONE = 7
} hal_clklf_source_t;

/** Function selecting whether XOSC16 should run during register retention power down mode.
 *
 * @param on Boolean value. True = on, false = off.
 */
void hal_clk_regret_xosc16m_on(bool on);

/** Function selecting the main source for the 16MHz system clock.
 *
 * @param input selects the clock source.
 * @arg HAL_CLK_XOSC16_OR_RCOSC16
 * @arg HAL_CLK_PAD_XC1
 */
void hal_clk_set_input(hal_clk_input_t input);

/** Function selecting the 16Mhz clock to be used. For this function to have any
 * effect the main source for the 16MHz clock must be either XOSC16 or RCOSC16. See
 * hal_clk_set_input().
 *
 * @param source selects the 16M clock to be used.
 * @arg HAL_CLK_XOSC16M_AND_RCOSC16M. Here, the source will be automatically be switched to
 * XOSC16M when possible (ready).
 * @arg HAL_CLK_RCOSC16M_ONLY
 * @arg HAL_CLK_XCOSC16M_ONLY
 */
void hal_clk_set_16m_source(hal_clk_source_t source);

/** Function returning the source for the 16MHz clock. For this function to have any
 * effect the main source for the 16MHz clock must be either XOSC16 or RCOSC16. See
 * hal_clk_set_input().
 * @note If you are using nRF24LE1 rev A you must change the SFR that is used to CLKCTRL
 */
hal_clk_source_t hal_clk_get_16m_source(void);

/** Function for setting the internal system clock frequency. The system clock source is
 * always 16MHz. This function configures the system clock prescaler.
 * @param freq selects the internal clock to be used.
 * @arg HAL_CLK_16MHZ
 * @arg HAL_CLK_8MHZ
 * @arg HAL_CLK_4MHZ
 * @arg HAL_CLK_2MHZ
 * @arg HAL_CLK_1MHZ
 * @arg HAL_CLK_500KHZ
 * @arg HAL_CLK_250KHZ
 * @arg HAL_CLK_125KHZ
*/
void hal_clk_set_freq(hal_clk_freq_t freq);

/**
 * Read the CLKLF (phase).
*/
bool hal_clklf_phase(void);

/**
 * Checks if the 32kHz clock is ready to be used.
*/
bool hal_clklf_ready(void);

/** Function for setting the source for the 32KHz clock (clklf / RTC).
 * @param source selects the 32KHz clock source.
 * @arg HAL_CLKLF_XOSC32K
 * @arg HAL_CLKLF_RCOSC32K
 * @arg HAL_CLKLF_XOSC16M_SYNTH
 * @arg HAL_CLKLF_PAD_XC1
 * @arg HAL_CLKLF_DIG_IO_RAIL
*/
void hal_clklf_set_source(hal_clklf_source_t source);

#endif // HAL_CLK_H__
/** @} */
