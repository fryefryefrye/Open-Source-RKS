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
 * @brief Interface functions for the analog-to-digital converter (ADC).
 * @defgroup hal_nrf24le1_hal_adc Analog to Digital Converter (hal_adc)
 * @{
 * @ingroup hal_nrf24le1
 *
 * This is a general-purpose ADC with up to 14 input channels. The ADC contains 
 * an internal 1.2V reference, but can also be used with external reference or 
 * full scale range equal to VDD. It can be operated in a single step mode with 
 * sampling under software control, or a continuous conversion mode with a programmable 
 * sampling rate.
 *
 * This module contains setup functions for configuring the ADC; operation functions 
 * for starting the ADC and reading a sampled value; and status functions for checking 
 *if the ADC is busy or ready, or has overflowed.
 */

/**
 * The following example show a typical usage of the ADC:
 * @code
 * void main(void)
 * {
 *   uint16_t adc_out;
 *
 *   adc_init();                                         // Initialise ADC
 *
 *   while(true)                                         // Endless loop
 *   {
 *     hal_adc_start();                                  // Start ADconversion
 *                                                     
 *     while(hal_adc_busy());                            // Wait for data ready
 *        
 *     adc_out = hal_adc_read_LSB();                     // Read LSB of output
 *     adc_out += ((uint16_t)hal_adc_read_MSB()) << 8;   // Add MSB of output
 * 
 *     // Output from ADC is now stored in adc_out
 *
 *   }
 * }
 *
 * // Initialize ADC: Set input channel to AIN7 and VDD as reference. Use 
 * // single ended mode and single step conversion. Use 10 bits to represent
 * // the analog input signal, set the output data to be right justified 
 *
 * void adc_init(void)
 * {
 *   hal_adc_set_input_channel(HAL_ADC_INP_AIN7);                     
 *   hal_adc_set_reference(HAL_ADC_REF_VDD);                        
 *   hal_adc_set_input_mode(HAL_ADC_SINGLE);                             
 *   hal_adc_set_conversion_mode(HAL_ADC_SINGLE_STEP);               
 *   hal_adc_set_resolution(HAL_ADC_RES_10BIT);                          
 *   hal_adc_set_data_just(HAL_ADC_JUST_RIGHT);    
 * }
 * @endcode
 */

#ifndef HAL_ADC_H__
#define HAL_ADC_H__

#include <stdint.h>
#include <stdbool.h>

#define ADC_STARTUP_CNT 1;

#define HAL_INP_AIN0   0x00
#define HAL_INP_AIN1   0x01
#define HAL_INP_AIN2   0x02
#define HAL_INP_AIN3   0x03
#define HAL_INP_AIN4   0x04
#define HAL_INP_AIN5   0x05
#define HAL_INP_AIN6   0x06
#define HAL_INP_AIN7   0x07
#define HAL_INP_AIN8   0x08
#define HAL_INP_AIN9   0x09 
#define HAL_INP_AIN10  0x0A
#define HAL_INP_AIN11  0x0B
#define HAL_INP_AIN12  0x0C
#define HAL_INP_AIN13  0x0D
#define HAL_INP_VDD1_3 0x0E
#define HAL_INP_VDD2_3 0x0F

/** An enum describing the ADC's input channel.
 *
 */
typedef enum {
  HAL_ADC_INP_AIN0   = HAL_INP_AIN0,
  HAL_ADC_INP_AIN1   = HAL_INP_AIN1,
  HAL_ADC_INP_AIN2   = HAL_INP_AIN2,
  HAL_ADC_INP_AIN3   = HAL_INP_AIN3,
  HAL_ADC_INP_AIN4   = HAL_INP_AIN4,
  HAL_ADC_INP_AIN5   = HAL_INP_AIN5,
  HAL_ADC_INP_AIN6   = HAL_INP_AIN6,
  HAL_ADC_INP_AIN7   = HAL_INP_AIN7,
  HAL_ADC_INP_AIN8   = HAL_INP_AIN8,
  HAL_ADC_INP_AIN9   = HAL_INP_AIN9, 
  HAL_ADC_INP_AIN10  = HAL_INP_AIN10,
  HAL_ADC_INP_AIN11  = HAL_INP_AIN11,
  HAL_ADC_INP_AIN12  = HAL_INP_AIN12,
  HAL_ADC_INP_AIN13  = HAL_INP_AIN13,
  HAL_ADC_INP_VDD1_3 = HAL_INP_VDD1_3,
  HAL_ADC_INP_VDD2_3 = HAL_INP_VDD2_3
} hal_adc_input_channel_t;

/** An enum describing the ADC's reference.
 *
 */
typedef enum {
  HAL_ADC_REF_INT  = 0x00,    /**< Internal 1.22V reference */
  HAL_ADC_REF_VDD  = 0x01,    /**< VDD as reference */
  HAL_ADC_REF_AIN3 = 0x02,    /**< External reference on AIN3 */
  HAL_ADC_REF_AIN9 = 0x03     /**< External reference on AIN9 */
} hal_adc_reference_t;

/** An enum describing the ADC's single ended or differential mode.
 *
 */
typedef enum {
  HAL_ADC_SINGLE    = 0x00,   /**< Single ended mode */
  HAL_ADC_DIFF_AIN2 = 0x01,   /**< Differential with AIN2 as inv. input */
  HAL_ADC_DIFF_AIN6 = 0x02    /**< Differential with AIN6 as inv. input */
} hal_adc_input_mode_t;

/** An enum describing the ADC's conversion mode.
 *
 */
typedef enum {
  HAL_ADC_SINGLE_STEP = 0x00, /**< Single step conversion */
  HAL_ADC_CONTINOUS   = 0x01  /**< Cont. conversion, def. sampling rate */
} hal_adc_conversion_mode_t;

/** An enum describing the ADC's sampling rate (Continuous conversion).
 *
 */
typedef enum {
  HAL_ADC_2KSPS  = 0x00,      /**< ADC sampling rate = 2kbps */
  HAL_ADC_4KSPS  = 0x01,      /**< ADC sampling rate = 4kbps */
  HAL_ADC_8KSPS  = 0x02,      /**< ADC sampling rate = 8kbps */
  HAL_ADC_16KSPS = 0x03       /**< ADC sampling rate = 16kbps */
} hal_adc_sampling_rate_t;

/** An enum describing the ADC's power down delay (Single step conversion).
 *
 */
typedef enum {               
  HAL_ADC_PDD_0US  = 0x00,    /**< ADC power down delay = 0us */  
  HAL_ADC_PDD_6US  = 0x01,    /**< ADC power down delay = 6us */
  HAL_ADC_PDD_24US = 0x02,    /**< ADC power down delay = 24us */
  HAL_ADC_PDD_INF  = 0x03     /**< ADC power down delay = infinite */
} hal_adc_power_down_delay_t;

/** An enum describing the ADC's input acquisition window.
 *
 */
typedef enum {
  HAL_ADC_AQW_075US = 0x00,   /**< Input acquisition window = 0.75us */
  HAL_ADC_AQW_3US   = 0x01,   /**< Input acquisition window = 3us */
  HAL_ADC_AQW_12US  = 0x02,   /**< Input acquisition window = 12us */
  HAL_ADC_AQW_36US  = 0x03    /**< Input acquisition window = 36us */
} hal_adc_acq_window_t;

/** An enum describing the ADC's resolution.
 *
 */
typedef enum {
  HAL_ADC_RES_6BIT  = 0x00,   /**< ADC resolution set to 6 bit */
  HAL_ADC_RES_8BIT  = 0x01,   /**< ADC resolution set to 8 bit */
  HAL_ADC_RES_10BIT = 0x02,   /**< ADC resolution set to 10 bit */
  HAL_ADC_RES_12BIT = 0x03    /**< ADC resolution set to 12 bit */
} hal_adc_resolution_t;

/** An enum describing the position of output data.
 *
 */
typedef enum {
  HAL_ADC_JUST_LEFT  = 0x00,/**< Left justified data */
  HAL_ADC_JUST_RIGHT = 0x01/**< Right justified data */
} hal_adc_data_just_t;

/** An enum describing the ADC's resolution.
 *
 */
typedef enum {
  HAL_ADC_FLOW_NONE  = 0x00,   /**< No overflow or underflow */
  HAL_ADC_FLOW_OVER  = 0x01,   /**< Overflow */
  HAL_ADC_FLOW_UNDER = 0x02,   /**< Underflow */
  HAL_ADC_FLOW_BOTH  = 0x03    /**< Both overflow and underflow */
} hal_adc_overflow_t;

/** @name   SETUP FUNCTIONS  */
//@{
/* Setup function prototypes */

/** Function to set the input channel for the ADC.
 * This function sets the input channel for the ADC.
 * 
 * @param chsel Input channel
 */
void hal_adc_set_input_channel(hal_adc_input_channel_t chsel);

/** Function to set the reference for ADC
 * This function sets the reference for conversion.
 * 
 * @param refsel Reference
 */
void hal_adc_set_reference(hal_adc_reference_t refsel);

/** Function to set the ADC single ended or differential mode.
 * This function sets the single ended or differential mode of the ADC.
 * 
 * @param input_mode Single ended or differential mode
 */
void hal_adc_set_input_mode(hal_adc_input_mode_t input_mode);

/** Function to set the ADC conversion mode.
 * This function sets the conversion mode of the ADC conversion.
 * 
 * @param conv_mode The conversion mode
 */
void hal_adc_set_conversion_mode(hal_adc_conversion_mode_t conv_mode);

/** Function to set the ADC sampling rate.
 * This function sets the sampling rate for the ADC.
 * 
 * @param rate The sampling rate
 */
void hal_adc_set_sampling_rate(hal_adc_sampling_rate_t rate);

/** Function to set the ADC power down delay.
 * This function sets the power down delay of the ADC.
 * 
 * @param pdd The power down delay
 */
void hal_adc_set_power_down_delay(hal_adc_power_down_delay_t pdd);

/** Function to set the duration of the acquisition window for the ADC.
 * This function sets the duration of the acquisition window.
 * 
 * @param tacq Duration of the acquisition window
 */
void hal_adc_set_acq_window(hal_adc_acq_window_t tacq);

/** Function to set the ADC resolution.
 * This function sets the resolution of the ADC conversion.
 * 
 * @param res The resolution of the ADC
 */
void hal_adc_set_resolution(hal_adc_resolution_t res);

/** Function to set the ADC data output justification.
 * This function sets the ADC output to be left or right justified.
 * 
 * @param just The resolution of the ADC
 */
void hal_adc_set_data_just(hal_adc_data_just_t just);
//@}


/** @name   OPERATION FUNCTIONS  */
//@{
/* Operation function prototypes */

/** Function to start the ADC conversion.
 * This function starts the conversion.
 */
void hal_adc_start(void);

/** Function that returns the LSB of the data from the ADC.
 * This function returns the LSB of the current data stored in the ADC.
 *
 * @return LSB of data from the ADC
 */
uint8_t hal_adc_read_LSB(void);

/** Function that returns the MSB of the data from the ADC.
 * This function returns the MSB of the current data stored in the ADC.
 *
 * @return MSB of data from the ADC
 */
uint8_t hal_adc_read_MSB(void);
//@}

/** @name   STATUS FUNCTIONS  */
//@{
/* Status functions prototypes */

/** Function that returns the status of the ADC.
 * This function returns true if the ADC is busy.
 *
 * @return ADC busy bit
 * @retval FALSE no conversion in progress
 * @retval TRUE conversion in progress
 */
bool hal_adc_busy(void);

/** Function that returns the flow status of the ADC.
 * Use this function to check for overflow and/or underflow.
 *
 * @return Overflow status
 */
hal_adc_overflow_t hal_adc_get_overflow_status(void);
//@}

#endif // HAL_ADC_H__
/**  @}  */

