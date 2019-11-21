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
 * @brief Implementation of hal_ancmp
 */

#include <stdint.h>
#include <stdbool.h>

#include "nrf24le1.h"
#include "nordic_common.h"
#include "hal_ancmp.h"

void hal_ancmp_set_polarity(hal_ancmp_pol_t polarity)
{ // Update "polarity" bit
  COMPCON = (COMPCON & 0xEF) | (((uint8_t)polarity << 4) & ~0xEF);   
}

void hal_ancmp_set_ref_voltage_scale(hal_ancmp_ref_scale_t scale)
{ // Update "refscale" bits
  COMPCON = (COMPCON & 0xF3) | (((uint8_t)scale << 2) & ~0xF3);   
}

void hal_ancmp_set_reference(hal_ancmp_ref_t ref)
{ // Update "cmpref" bit
  COMPCON = (COMPCON & 0xFD) | (((uint8_t)ref << 1) & ~0xFD);   
}

void hal_ancmp_set_input_channel(hal_ancmp_input_channel_t chsel)
{ 
  hal_adc_input_channel_t channel;
  channel = (hal_adc_input_channel_t)chsel;
  hal_adc_set_input_channel(channel);       
}

void hal_ancmp_enable(bool en)
{
  if(en)
  {
    COMPCON = COMPCON | BIT_0;                // Set "enable" bit
  }
  else
  {
    COMPCON = COMPCON & ~BIT_0;               // Clear "enable" bit
  }
}