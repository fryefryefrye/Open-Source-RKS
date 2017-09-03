#ifndef __HAL_DELAY_H
#define __HAL_DELAY_H 
#include "stdint.h"

/* This function overflows at 65535 us. 
 * For Keil C51 compiler, this function is accurate down to 10 us, with deviation of upto 2 us.
 *
 * For the IAR 8051 C compiler, the function is not as accurate before hitting about 500 us.
 * Over this number, it seem to have a deviation of about 5 to 10 us.
 */
void delay_us(uint16_t us);

/* Function for looping "delay_us"
 * 
 */
void delay_ms(uint16_t ms);

#define delay_s(x)  delay_ms((uint16_t)1000 * (uint16_t)x)
#endif
