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
 * $LastChangedRevision: 2503 $
 */

/** @file
 * @brief Implementation of hal_w2
 */

#include <stdint.h>
#include <stdbool.h>

#include "nrf24le1.h"
#include "nordic_common.h"
#include "hal_w2.h"
#include "hal_delay.h"

#define BROADCAST_ENABLE      7     // W2CON0 register bit 7
#define CLOCK_STOP            6     // W2CON0 register bit 6
#define X_STOP                5     // W2CON0 register bit 5
#define X_START               4     // W2CON0 register bit 4
#define CLOCK_FREQUENCY_1     3     // W2CON0 register bit 3
#define CLOCK_FREQUENCY_0     2     // W2CON0 register bit 2
#define MASTER_SELECT         1     // W2CON0 register bit 1
#define WIRE_2_ENABLE         0     // W2CON0 register bit 0

void hal_w2_soft_reset();

/* Slave specific functions */

void hal_w2_respond_to_gen_adr(bool resp_gen)
{ 
  if(resp_gen)
  {                                                                   
    W2CON0 = W2CON0 | (1 << BROADCAST_ENABLE);  // Set "broadcastEnable" bit
  }
  else
  {
    W2CON0 = W2CON0 & ~(1 << BROADCAST_ENABLE); // Clear "broadcastEnable" bit
  }
}

void hal_w2_alter_clock(bool alt_clk)
{
  if(alt_clk)                         
  {
    W2CON0 = W2CON0 | (1 << CLOCK_STOP);      // Set "clockStop" bit
  }
  else
  {
    W2CON0 = W2CON0 & ~(1 << CLOCK_STOP);     // Clear "clockStop" bit
  }
}

void hal_w2_irq_stop_cond_enable(bool stop_cond)
{ 
  if(stop_cond)
  {                                                                  
    W2CON0 = W2CON0 & ~(1 << X_STOP);         // Clear "xStop" bit
  }
  else
  {
    W2CON0 = W2CON0 | (1 << X_STOP);          // Set "xStop" bit
  }
}

void hal_w2_irq_adr_match_enable(bool addr_match)
{
  if(addr_match)
  {
    W2CON0 = W2CON0 & ~(1 << X_START);        // Clear "xStart" bit
  }
  else
  {
    W2CON0 = W2CON0 | (1 << X_START);         // Set "xStart" bit
  }
}

void hal_w2_set_slave_address(uint8_t address)
{
  W2SADR = (address & 0x7F);                  // Set 7 bit adress of the slave
}

/* General functions */

void hal_w2_set_clk_freq(hal_w2_clk_freq_t freq)
{                                             
  W2CON0 = (W2CON0 & 0xF3) | (((uint8_t)freq) << CLOCK_FREQUENCY_0);       
}                                             // Update "clockFrequency" bits

void hal_w2_set_op_mode(hal_w2_op_mode_t mode)
{
  if(mode == HAL_W2_MASTER)                   // Check for master mode
  {                                                                  
    W2CON0 = W2CON0 | (1 << MASTER_SELECT);   // Set "masterSelect" bit
  }
  else
  {
    W2CON0 = W2CON0 & ~(1 << MASTER_SELECT);  // Clear "masterSelect" bit
  }
}

void hal_w2_enable(bool en)
{ 
  if(en)
  {
    W2CON0 = W2CON0 | (1 << WIRE_2_ENABLE);   // Set "wire2Enable" bit
  }
  else
  {
    W2CON0 = W2CON0 & ~(1 << WIRE_2_ENABLE);  // Clear "wire2Enable" bit
  }
}

void hal_w2_all_irq_enable(bool irq)
{ /* In this function the standard "read-modify-write" is not used because
     bit 4:0 (the status bits) in W2CON1 are cleared when read. These bits
     are read only so they can not be modified. */
  if(irq)
  {
    W2CON1 = ~(BIT_5);                        // Clear "maskIrq" bit
  }
  else
  {
    W2CON1 = BIT_5;                           // Set "maskIrq" bit
  }
}

void hal_w2_configure_master(hal_w2_clk_freq_t mode)
{
  hal_w2_enable(true);
  hal_w2_set_clk_freq(mode);
  hal_w2_set_op_mode(HAL_W2_MASTER);

  INTEXP |= 0x04;                         // Enable 2 wire interrupts
  W2CON1 = 0x00;
  hal_w2_all_irq_enable(true);             // Enable interrupts in the 2-wire  
  SPIF = 0;
}

uint8_t hal_w2_wait_data_ready(void)
{
  uint32_t timeout_counter = 0x0FF;
  uint8_t w2_status;
  bool data_ready;
  bool nack_received;

  do
  {
    w2_status = W2CON1;
    data_ready = (w2_status & W2CON1_FLAG_DATA_READY);
    nack_received = (w2_status & W2CON1_FLAG_NACK);
    delay_us(10);
  } while (!data_ready);


  return w2_status;
}

bool hal_w2_init_transfer(uint8_t address, hal_w2_direction_t direction)
{
  uint8_t w2_status;

  HAL_W2_ISSUE_START_COND;
  HAL_W2_WRITE((address << 1) | (uint8_t)direction);

  w2_status = hal_w2_wait_data_ready();

  if (w2_status & W2CON1_FLAG_NACK)
  {
    return false; // NACK received from slave or timeout
  }
  else
  {
    return true; // ACK received from slave
  }
}

bool hal_w2_write(uint8_t address, const uint8_t *data_ptr, uint8_t data_len)
{
  bool ack_received;
  ack_received = hal_w2_init_transfer(address, HAL_W2_DIR_WRITE);

  while (data_len-- > 0 && ack_received == true)
  {
    uint8_t w2_status;
    HAL_W2_WRITE(*data_ptr++);
    w2_status = hal_w2_wait_data_ready();
    if (w2_status & W2CON1_FLAG_NACK)
    {
      ack_received = false;
    }
  }

  HAL_W2_ISSUE_STOP_COND;

  return ack_received;
}

bool hal_w2_read(uint8_t address, uint8_t *data_ptr, uint8_t data_len)
{
  uint8_t w2_status;
  bool ack_received;
  
  ack_received = hal_w2_init_transfer(address, HAL_W2_DIR_READ);

  if (ack_received == false) 
  {
    // This situation (NACK received on bus while trying to read from a slave) leads to a deadlock in the 2-wire interface. 
    hal_w2_soft_reset(); // Workaround for the deadlock
  }


  while (data_len-- && ack_received)
  {
    if (data_len == 0)
    {
      HAL_W2_ISSUE_STOP_COND;
    }

    w2_status = hal_w2_wait_data_ready();

    *data_ptr++ = HAL_W2_READ();
    ack_received = !(w2_status & W2CON1_FLAG_NACK);
  }

  return ack_received;  
}

void hal_w2_soft_reset()
{
#ifndef W2_SOFT_RESET_NOT_AVAILABLE
  uint8_t pulsecount, w2_freq;

  // Store the selected 2-wire frequency 
  w2_freq = W2CON0 & 0x0C;
  // Prepare the GPIO's to take over SDA & SCL
  HAL_W2_CLEAR_SDA_SCL;
  HAL_W2_OVERRIDE_SDA_SCL(1, 1);
  //P0DIR = 0xFF;
  
  // Reset 2-wire. SCL goes high.
  W2CON0 = 0x03;
  W2CON0 = 0x07;
  
  // Disable 2-wire.
  W2CON0 = 0x06;
  
  // SDA and SCL are now under software control, and both are high. 
  // Complete first SCL pulse.
  //P0DIR = 0xEF;
  HAL_W2_OVERRIDE_SDA_SCL(1, 0);
  
  // SCL low
  delay_us(5);
  //P0DIR = 0xCF;
  HAL_W2_OVERRIDE_SDA_SCL(0, 0);
  
  // SDA low
  // Create SCL pulses for 7 more data bits and ACK/NACK
  delay_us(5);
  for( pulsecount = 0; pulsecount < 8; pulsecount++ )
  {
    //P0DIR = 0xDF;
    HAL_W2_OVERRIDE_SDA_SCL(0, 1);
    delay_us(5);
    //P0DIR = 0xCF;
    HAL_W2_OVERRIDE_SDA_SCL(0, 0);
    delay_us(5);
  }
  
  // Generating stop condition by driving SCL high
  delay_us(5);
  //P0DIR = 0xDF;
  HAL_W2_OVERRIDE_SDA_SCL(0, 1);
  
  // Drive SDA high
  delay_us(5);
  //P0DIR = 0xFF;
  HAL_W2_OVERRIDE_SDA_SCL(1, 1);
  
  // Work-around done. Return control to 2-wire.
  W2CON0 = 0x07;
  
  // Reset 2-wire and return to master mode at the frequency selected before calling this function
  W2CON0 = 0x03;
  W2CON0 = 0x03 | w2_freq;
#endif
}
