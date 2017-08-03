/* Copyright (c) 2007 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT. 
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRENTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 * $LastChangedRevision: 186 $
 */

/** @file
 * Type definitions for firmware projects developed at Nordic Semiconductor.
 *
 * Standard storage classes in C, such as @c char, @c int, and @c long, are not always
 * interpreted in the same way by the compiler. The types here are defined by their
 * bit length and signed/unsigned property, as their names indicate. The correlation
 * between the name and properties of the storage class should be true, regardless of
 * the compiler being used.
 */

#ifndef __STDINT_H__
#define __STDINT_H__

#ifdef __C51__

typedef unsigned char uint8_t;        ///< 8 bit unsigned int

typedef signed char int8_t;          ///< 8 bit signed int

typedef unsigned int uint16_t;        ///< 16 bit unsigned int

typedef signed int int16_t;          ///< 16 bit signed int

typedef unsigned long uint32_t;       ///< 32 bit unsigned int

typedef signed long int32_t;         ///< 32 bit signed int

#endif // __C51__

#ifndef NULL
#define NULL (void*)0
#endif

#endif // __STDINT_H__
