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
 * $LastChangedRevision: 159 $
 */

/** @file
 * @brief Interface functions for encrypting data using the Advanced Encryption Standard (AES) 
 * hardware module(s).
 * @defgroup hal_nrf24le1_hal_aes AES encryption (hal_aes)
 * @{
 * @ingroup hal_nrf24le1
 * 
 * The nRF24LE1 contains a co-processor for computing an 8 by 8 Galois Field Multiplication 
 * with an 8 bits output. This is the polynomial used by AES (Advanced Encryption Standard). 
 *
 * The nRF24LU1 contains a full hardware implementation of the AES algorithm, supporting 
 * both encryption and decryption for the modes ECB, CTR, OFB, CFB and CBC.
 *
 * The AES HAL contains a common interface using the dedicated hardware implementation 
 * for the nRF24LU1 and a firmware implementation utilizing the Galois Multiplication 
 * co-processor for the nRF24LE1.
 *
 * <b>Note that for the nRF24LE1 implementation only ECB mode encryption is supported.</b>
 */

#ifndef HAL_AES_H__
#define HAL_AES_H__

#include <stdint.h>
#include <stdbool.h>

/**
Must be defined in order to enable the function aes_get_dec_key()
*/
#define _AES_ENABLE_GET_DEC_KEY_

/* Function parameter defines*/
typedef enum
{
  CBC,
  CFB,
  OFB,
  CTR,
  ECB,
} aes_modes_t;

/**
@remark
This interface is common to nRF24LU1 and nRF24LE1. For nRF24LE1 only <b>ECB</b> 
mode and <b>encryption</b> have been implemented. Other modes (described below) 
apply only to the nRF24LU1 implementation. This is because nRF24LU1 comes 
with built-in HW support for AES encryption in various modes. nRF24LE1 has only
limited support for AES where most of the implementation is done in SW. 

It configures the AES module prior to encrypting/decrypting with aes_crypt().
The parameters reside in memory, thus the function does not need to be run every 
time aes_crypt() is used.

@param decrypt_enable selects encrypt or decrypt operation. Possible arguments are:
@arg true selects decrypt operation
@arg false selects encrypt operation

@param mode selects AES mode of operation. Possible arguments are:
@arg @c ECB
@arg @c CTR
@arg @c OFB
@arg @c CFB
@arg @c CBC

@param keyin is a pointer to a 16-byte array containing the key to be used for
the cryptographic operation. Passing a zero pointer will leave the previous 
written key unaltered.

@param ivin is a pointer to a 16-byte array containing the initialization
vector needed for CTR, OFB, CFB and CBC mode of operation. Passing a zero 
pointer will leave the previous, written initialization vector unaltered.

@remarks

Which key is to be used for the cryptographic operation depends on the selected 
mode(ECB/CTR/OFB/CFB/CBC) and the direction of operation (encrypt/decrypt). For 
modes ECB and CBC the decrypt operation is different from the encrypt operation, 
while for the remaining modes the operation is identical for the two directions. 
As a result, when using ECB or CBC mode the key needed for decrypting a block of 
data is different from the key that was used for encrypting the block. However, 
for CTR, OFB and CFB the selected direction of operation does not care and the same 
key is used for both encryption and decryption.

The function aes_get_dec_key() can be used to aquire the decryption key from the 
encryption key when using ECB and CFB mode.

The initializtion vector is only loaded internally to the AES engine after one 
of the following occurrences:

-# A new initializtion vector is passed
-# Mode of operation is changed
-# In CBC mode when direction of operation is changed
*/

void hal_aes_setup(bool decrypt_enable, aes_modes_t mode, const uint8_t *keyin, const uint8_t *ivin);
/**

Encrypts or decrypts a 128 bit (16 byte) block. The cryptographic operation is 
configured by using aes_setup().

@param dest_buf is a pointer to the 16-byte destination array.
@param src_buf is a pointer to the 16-byte source array.

@remarks

@sa hal_aes_setup();
*/
void hal_aes_crypt(uint8_t *dest_buf, const uint8_t *src_buf);

/**
Calculates decryption key from encryption key.
For ECB and CBC mode of operation, different keys are used for encryption and 
decryption. This function calculates the required decryption key for these modes,
based on the encryption key.

@param dest_dec_key is a pointer to the 16-byte destination array where the 
calculated encryption key is to be placed.

@param src_enc_key is a pointer to the 16-byte source array containing the 
encryption key from which the decryption key is to be calculated.

@remarks
Before using this function @c _AES_ENABLE_GET_DEC_KEY_ must be defined.
@sa hal_aes_setup(), _AES_ENABLE_GET_DEC_KEY_
*/
void hal_aes_get_dec_key(uint8_t *dest_dec_key, const uint8_t *src_enc_key);

#endif  // AES_HAL_H__

/** @} */
