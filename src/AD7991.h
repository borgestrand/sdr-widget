/*
 * AD7991.h
 *
 *  Created on: 2010-02-21
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef AD7991_H_
#define AD7991_H_


// DEFS for the AD7991 4 x ADC chip
// This  chip comes in two versions
// AD7991-500 has I2C address 0x28
// AD7991-1500 has I2C address 0x29
// Normal address 0x28
#define AD7991_I2C_ADDRESS	0x28
#define AD7991_PA_CURRENT	0
#define AD7991_POWER_OUT	1
#define AD7991_POWER_REF	2
#define AD7991_PSU_VOLTAGE	3

extern uint16_t	ad7991_adc[4];					// Last measured values read from the AD7991 ADC
												// all values adjusted for a full scale 16 bit unsigned int
extern uint8_t ad7991_setup(uint8_t i2c_address);
extern uint8_t ad7991_poll(uint8_t i2c_address);

#endif /* AD7991_H_ */
