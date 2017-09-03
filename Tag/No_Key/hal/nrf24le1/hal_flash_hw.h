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
 * @brief Header file defining flash parameters for nRF24LE1
 * @addtogroup hal_nrf24le1_hal_flash
 * @{
 * @name Hardware dependencies
 * This sections contains hardware specific flash definitions. These definitions
 * are not used inside the Flash HAL directly, but can become handy for the 
 * user of the Flash HAL.   
 * @{
 *
 */

#ifndef HAL_FLASH_LE1_H__
#define HAL_FLASH_LE1_H__

/**
Number of bytes per Flash page.
*/
#define HAL_FLASH_PAGE_SIZE 512

/**
Start (xdata) address for "Non Voltaile Data Memory".
 
Used by the EEPROM library.
*/
#define HAL_DATA_NV_BASE_ADDRESS 0xFA00 

/**
Defines the number of physical Flash pages used by one  
"Non Voltaile Data Memory" page. 
*/
#define HAL_DATA_NV_FLASH_PAGES 2

/**
Defines the first physical flash page to which one 
"Non Voltaile Data Memory" is mapped.

Used by the EEPROM library. 
*/
#define HAL_DATA_NV_FLASH_PN0 32

/**
Defines the second physical flash page to which one 
"Non Voltaile Data Memory" page is mapped.

Used by the EEPROM library. 
*/
#define HAL_DATA_NV_FLASH_PN1 33

#endif // HAL_FLASH_LE1_H__
/** @} */
/** @} */

