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
@brief Interface for self-programming of on-chip Flash / Non Volatile Data Memory.
@defgroup hal_nrf24le1_hal_flash Flash (hal_flash)
@{
@ingroup hal_nrf24le1


@details For nRF24LE1 there are 2 options for flash writing which is selected
by the PMW bit in PCON:
 - PMW == 1: Program memory
 - PMW == 1: NV Data Memory
Example:
@code
#include "hal_flash.h"
#define PMW 0x10

PCON |= PMW;

// Writing to the Program Memory
hal_flashbyte_write_byte(0xFA00,0xCD);

PCON &= ~PMW;

// Writing to the NV Data Memory
hal_flashbyte_write_byte(0x2000,0xEF);

@endcode

@attention Flash pages must be erased before they are written to. This is not
shown in the example.

@sa lib_eeprom

 */
#ifndef HAL_FLASH_H__
#define HAL_FLASH_H__

#include <stdint.h>
#include "hal_flash_hw.h"

#define PMW  0x10
/** Function to erase a page in the Flash memory
 *  @param pn Page number
 */
void hal_flash_page_erase(uint8_t pn);

/** Function to write a byte to the Flash memory
 *  @param a 16 bit address in Flash
 *  @param b byte to write
 */
void hal_flash_byte_write(uint16_t a, uint8_t b);

/** Function to write n bytes to the Flash memory
 *  @param a 16 bit address in Flash
 *  @param *p pointer to bytes to write
 *  @param n number of bytes to write
 */
void hal_flash_bytes_write(uint16_t a, const uint8_t *p, uint16_t n);

/** Function to read a byte from the Flash memory
 *  @param a 16 bit address in Flash
 *  @return the byte read
 */
uint8_t hal_flash_byte_read(uint16_t a);

/** Function to read n bytes from the Flash memory
 *  @param a 16 bit address in Flash
 *  @param *p pointer to bytes to write
 *  @param n number of bytes to read
 */
void hal_flash_bytes_read(uint16_t a, uint8_t *p, uint16_t n);

#endif // HAL_FLASH_H__
/** @} */


