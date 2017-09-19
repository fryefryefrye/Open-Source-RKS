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
 *@brief Implementation of hal_flash
 */

#include "nrf24le1.h"
#include "hal_flash.h"

void hal_flash_page_erase(uint8_t pn)
{ 
  // Save interrupt enable state and disable interrupts:
  F0 = EA;
  EA = 0;

  // Enable flash write operation:
  WEN = 1;
 
  // Write the page address to FCR to start the page erase operation. This
  // operation is "self timed" when executing from the flash; the CPU will
  // halt until the operation is finished:
  FCR = pn;

  // When running from XDATA RAM we need to wait for the operation to finish:
  while(RDYN == 1)
    ;
	
  WEN = 0;
  
  EA = F0; // Restore interrupt enable state  
}

void hal_flash_byte_write(uint16_t a, uint8_t b)
{
  static uint8_t xdata *pb;
    
  // Save interrupt enable state and disable interrupts:
  F0 = EA;
  EA = 0;
  
  // Enable flash write operation:
  WEN = 1;
  
  // Write the byte directly to the flash. This operation is "self timed" when
  // executing from the flash; the CPU will halt until the operation is
  // finished:
  pb = (uint8_t xdata *)a;
  *pb = b; //lint -e613 "Null pointer ok here"

  // When running from XDATA RAM we need to wait for the operation to finish:
  while(RDYN == 1)
    ;

  WEN = 0;

  EA = F0; // Restore interrupt enable state
}

void hal_flash_bytes_write(uint16_t a, const uint8_t *p, uint16_t n)
{
  static uint8_t xdata *pb;

  // Save interrupt enable state and disable interrupts:
  F0 = EA;
  EA = 0;

  // Enable flash write operation:
  WEN = 1;

  // Write the bytes directly to the flash. This operation is
  // "self timed"; the CPU will halt until the operation is
  // finished:
  pb = (uint8_t xdata *)a;
  while(n--)
  {
    *pb++ = *p++;
    //
    // When running from XDATA RAM we need to wait for the operation to
    // finish:
    while(RDYN == 1)
      ;
  }

  WEN = 0;

  EA = F0; // Restore interrupt enable state
}

uint8_t hal_flash_byte_read(uint16_t a)
{
  uint8_t xdata *pb = (uint8_t xdata *)a;
  return *pb;
}

void hal_flash_bytes_read(uint16_t a, uint8_t *p, uint16_t n)
{  
  uint8_t xdata *pb = (uint8_t xdata *)a;
  while(n--)
  {
    *p = *pb;
    pb++;
    p++;
  }
}
