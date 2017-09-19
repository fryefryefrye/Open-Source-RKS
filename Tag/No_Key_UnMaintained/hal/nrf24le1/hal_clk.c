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
 * @brief Implementation of hal_clk
 */

#include "nrf24le1.h"
#include <stdbool.h>
#include "hal_clk.h"

void hal_clk_regret_xosc16m_on(bool on)
{
  if(on)
  {
    CLKCTRL = (CLKCTRL | 0x80U) & (uint8_t)~0x08U;   // & ~0x08 to prevent writing 1 to this bit
  }
  else
  {
    CLKCTRL = CLKCTRL & (uint8_t)~0x88U;            // & ~0x08 to prevent writing 1 to this bit
  }
}

void hal_clk_set_input(hal_clk_input_t input)
{
  CLKCTRL = (CLKCTRL & (uint8_t)~0x48U) | ((input & 0x01U) << 6);
}

void hal_clk_set_16m_source(hal_clk_source_t source)
{
  CLKCTRL = (CLKCTRL & (uint8_t)~0x38U) | ((source & 0x03U) << 4);
}

hal_clk_source_t hal_clk_get_16m_source(void)
{
  hal_clk_source_t clock_source;
  if(CLKLFCTRL & (uint8_t)0x08U)
  {
    clock_source = HAL_CLK_XOSC16M;
  }
  else
  {
    clock_source = HAL_CLK_RCOSC16M;
  }

  return clock_source;
}

void hal_clk_set_freq(hal_clk_freq_t freq)
{
  CLKCTRL = ((CLKCTRL & (uint8_t)~0x0FU) | ((uint8_t)freq & 0x07U)); // & ~0x08 to prevent writing 1 to this bit
}

bool hal_clklf_phase(void)
{
  return (bool)(CLKLFCTRL & 0x80U);
}

bool hal_clklf_ready(void)
{
  return (bool)(CLKLFCTRL & 0x40U);
}

void hal_clklf_set_source(hal_clklf_source_t source)
{
  CLKLFCTRL = (uint8_t)(CLKLFCTRL & (uint8_t)~0x07U) | (uint8_t)((uint8_t)source & (uint8_t)0x07U);
}
