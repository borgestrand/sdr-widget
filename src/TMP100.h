/*
 * TMP100.h
 *
 *  Created on: 2010-02-21
 *      Author: TF3LJ
 */

#ifndef TMP100_H_
#define TMP100_H_

#include "I2C.h"

// DEFS for the TMP100 chip
// I2C Addresses for this chip can be:
// 0x48. 0x4a, 0x4c, 0x0x4e
// Normal address is 0x4e
#define TMP100_I2C_ADDRESS	0x4e
// default mode is 9 bit resolution, could be set at 12 bit resolution, but no need
//     0 =    0 deg C
// 32767 =  128 deg C
// 32768 = -128 deg C

// A global containing the last measured value read from the TMP100 temperature sensor
extern int16_t tmp100_data;

extern uint8_t tmp100_init(uint8_t i2c_address);
extern uint8_t tmp100_read(uint8_t i2c_address);

#endif /* TMP100_H_ */
