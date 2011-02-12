/*
 * PCF8574.h
 *
 *  Created on: 2010-02-27
 *      Author: TF3LJ
 */

#ifndef _PCF8574_h_
#define _PCF8574_h_

#include "I2C.h"

//Defs for onboard PCF8574 chip
// I2C Addresses for this chip can be:
// If NXP 8574P,  then: 0x20, 0x21. 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
// If NXP 8574Ax, then: 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
// Normal address is 0x3f
#define PCF_MOBO_I2C_ADDR	0x3f			// I2C address for the onboard PCF8574
#define Mobo_PCF_FILTER		(3 << 0)		// First bit of three consecutive for BPF
//#define MoboPCFUndefined1	(1 << 3)		// 8bit BCD or M0RZF style for LPF switching
//#define MoboPCFUndefined2	(1 << 4)		// 8bit BCD or M0RZF style for LPF switching
//#define MoboPCFUndefined3	(1 << 5)		// 8bit BCD or M0RZF style for LPF switching
#define Mobo_PCF_TX2		(1 << 6)		// SWR Protect secondary PTT
#define Mobo_PCF_TX			(1 << 7)		// TX PTT, active low

#if SCRAMBLED_FILTERS						// Enable a non contiguous order of filters
// Default filter addresses for Band Pass filters
#define Mobo_PCF_FLT0		0x00			// Filter setting is a 3 bit value
#define Mobo_PCF_FLT1		0x01			// Filter order could be scrambled if desired
#define Mobo_PCF_FLT2		0x02			// Can be read/modified through
#define Mobo_PCF_FLT3		0x03			// USB commands 0x18/19 (see Readme.txt file)
#define Mobo_PCF_FLT4		0x04
#define Mobo_PCF_FLT5		0x05
#define Mobo_PCF_FLT6		0x06
#define Mobo_PCF_FLT7		0x07
#endif


//Defs for two offboard PCF8574A chips in the MegaFilter Mobo
// I2C Addresses for these chips can be:
// If NXP 8574P,  then: 0x20, 0x21. 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
// If NXP 8574Ax, then: 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
// Normal addresses are 0x39 and 0x3a

#define PCF_LPF1_I2C_ADDR	0x39			// I2C address for first Megafilter Mobo PCF8574
#define PCF_LPF2_I2C_ADDR	0x3a			// I2C address for second Megafilter Mobo PCF8574


#if SCRAMBLED_FILTERS						// Enable a non contiguous order of filters
// Default filter addresses for Band Pass filters
#define I2C_EXTERN_FLT0		0x00			// Filter setting is a 4bit value
#define I2C_EXTERN_FLT1		0x01			// Filter order could be scrambled if desired
#define I2C_EXTERN_FLT2		0x02			// Can be read/modified through
#define I2C_EXTERN_FLT3		0x03			// USB commands 0x1a/1b (see Readme.txt file)
#define I2C_EXTERN_FLT4		0x04
#define I2C_EXTERN_FLT5		0x05
#define I2C_EXTERN_FLT6		0x06
#define I2C_EXTERN_FLT7		0x07
#define I2C_EXTERN_FLT8		0x08
#define I2C_EXTERN_FLT9		0x09
#define I2C_EXTERN_FLTa		0x0a
#define I2C_EXTERN_FLTb		0x0b
#define I2C_EXTERN_FLTc		0x0c
#define I2C_EXTERN_FLTd		0x0d
#define I2C_EXTERN_FLTe		0x0e
#define I2C_EXTERN_FLTf		0x0f
#endif


// pcf_data_out contains the current output data on the builtin PCF8574 on the Mobo
extern uint8_t	pcf8574_mobo_data_out;

/*! \brief Set output bits in the PCF8574 built into to the Mobo 4.3
 *
 * \retval I2C status.
 */
extern uint8_t pcf8574_mobo_set(uint8_t i2c_address, uint8_t byte);

/*! \brief Clear output bits in the PCF8574 built into to the Mobo 4.3
 *
 * \retval I2C status.
 */
extern uint8_t pcf8574_mobo_clear(uint8_t i2c_address, uint8_t byte);

/*! \brief Write all 8 bits to a PCF8574
 *
 * \retval I2C status.
 */
extern uint8_t pcf8574_out_byte(uint8_t i2c_address, uint8_t data);

/*! \brief Read all 8 input bits from a PCF8574
 *
 * \retval PCF8574 input register (uint8_t)
 */
extern uint8_t pcf8574_in_byte(uint8_t i2c_address, uint8_t *data_to_return);

#endif
