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
 * @brief Implementation of hal_pof
 */

#include "nrf24le1.h"
#include "hal_pof.h"

void hal_pof_enable(bool enable)
{
	if(enable)
		POFCON |= 0x80;
	else
		POFCON &= ~0x80;
}

void hal_pof_set_threshold(hal_pof_threshold_t threshold)
{
	POFCON &= ~0x60;
	POFCON |= ((uint8_t)threshold & 0x60);
}

bool hal_pof_warning(void)
{
	if(POFCON & 0x10)
		return true;
	else
		return false;
}
