/*
 * AD5301.h
 *
 *  Created on: 2010-02-21
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef AD5301_H_
#define AD5301_H_

#include "I2C.h"

// DEFS for the AD5301 DAC chip
// I2C Addresses for this chip can be:
// 0x0c or 0x0d
// Normal address is 0x0d
#define AD5301_I2C_ADDRESS	0x0d


extern uint8_t ad5301(uint8_t i2c_address, uint8_t value);

#endif /* AD5301_H_ */
