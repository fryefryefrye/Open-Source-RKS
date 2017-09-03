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
 * @brief Implementation of  hal_spi
 */

#include "nrf24le1.h"
#include "hal_spi.h"
#include "nordic_common.h"

void hal_spi_master_init(hal_spi_clkdivider_t ck, hal_spi_mode_t mode, hal_spi_byte_order_t bo)
{
  SPIMCON0 = 0;                           // Default register settings
  switch (ck)                             // Set desired clock divider
  {
    case SPI_CLK_DIV2:
      SPIMCON0 |= (0x00 << 4);
      break;
    case SPI_CLK_DIV4:
      SPIMCON0 |= (0x01 << 4);
      break;
    case SPI_CLK_DIV8:
      SPIMCON0 |= (0x02 << 4);
      break;
    case SPI_CLK_DIV16:
      SPIMCON0 |= (0x03 << 4);
      break;
    case SPI_CLK_DIV32:
      SPIMCON0 |= (0x04 << 4);
      break;
    case SPI_CLK_DIV64:                   // We use clock divder 64 as default
    default:
      SPIMCON0 |= (0x05 << 4);
      break;  
  }
  switch(mode)                            // Set desired mode
  {
    case HAL_SPI_MODE_0:
      SPIMCON0 |= (0x00 << 1);
      break;
    case HAL_SPI_MODE_1:
      SPIMCON0 |= (0x01 << 1);
      break;
    case HAL_SPI_MODE_2:
      SPIMCON0 |= (0x02 << 1);
      break;
    case HAL_SPI_MODE_3:      
      SPIMCON0 |= (0x03 << 1);
      break;
  }
  
  if(bo == HAL_SPI_LSB_MSB)               // Set desired data order
  {
    SPIMCON0 |= BIT_3;
  }

  SPIMCON0 |= BIT_0;                      // Enable SPI master
}

uint8_t hal_spi_master_read_write(uint8_t pLoad)
{
  SPIMDAT = pLoad ;                       // Write data to SPI master
  while(!(SPIMSTAT & 0x04))               // Wait for data available in rx_fifo
  ;
  return SPIMDAT;                         // Return data register
}

void hal_spi_slave_init(hal_spi_mode_t mode, hal_spi_byte_order_t byte_order)
{
  uint8_t temp;
  SPISCON0 = 0xF0; //default register settings
  I3FR=1; 
  INTEXP |= 0x01; //gate SPI slave interrupt to INT3
  
  switch(mode)
  {
    case 0:
      SPISCON0 |= 0;
      break;
    case 1:
      SPISCON0 |= (1 << 1);
      break;
    case 2:
      SPISCON0 |= (2 << 1);
      break;
    case 3:      
      SPISCON0 |= (3 << 1);
      break;
  }
  SPISCON0 |= (~(byte_order & 0x01)) << 3; 

  //enable SPI slave
  SPISCON0 |= 0x01;
  while((SPISSTAT & 0x01))
  temp=SPISDAT;	//flush rx fifo
}

uint8_t hal_spi_slave_rw(uint8_t pLoad)
{
  hal_spi_slave_preload(pLoad);
  return hal_spi_slave_read();

}

uint8_t sstat_shadow = 0;

bool hal_spi_slave_csn_high()
{
  static bool csn_high = true;
  sstat_shadow |= SPISSTAT;

  if(sstat_shadow & 0x20)
  {
    csn_high = true;  
  }
  else
  {
    if(sstat_shadow & 0x10)
    {
      csn_high = false;
    }
  }
  sstat_shadow &= ~0x30;
  
  return csn_high;
}

bool spi_slave_data_ready()
{
  sstat_shadow |= SPISSTAT;

  if(sstat_shadow & 0x01)
  {
    return true;  
  }
  else
  {
    return false;
  }   
}

uint8_t hal_spi_slave_read()
{
  while(!(sstat_shadow & 0x01))
  {
    sstat_shadow |= SPISSTAT;
  }
  sstat_shadow &= ~0x01;

  return SPISDAT;
}

void hal_spi_slave_preload(uint8_t pLoad)
{
  SPISDAT=pLoad;
}
