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
 * $LastChangedRevision: 230 $
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

#ifndef __STDBOOL_H__
#define __STDBOOL_H__

//lint -strong(b,_Bool)
typedef unsigned char _Bool; ///< Boolean type

#define bool _Bool
#define true ((bool)1)
#define false ((bool)0)
#define __bool_true_false_are_defined 1

#endif // __STDBOOL_H__
