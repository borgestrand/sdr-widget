/*
 * Si570.h
 *
 *  Created on: 2010-02-27
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef SI570_H_
#define SI570_H_


// DEFS for the Si570 chip
// I2C Address for this chip is normally 0x55
#define	SI570_I2C_ADDRESS	0x55			// Default Si570 I2C address (can change per device)
#define	DEVICE_XTAL			0x7248F5C2		// 114.285 * _2(24)
#define	DCO_MIN				4850			// min VCO frequency 4850 MHz
#define DCO_MAX				5670			// max VCO frequency 5670 MHz

extern uint8_t		si570reg[];				// Si570 Registers

extern uint8_t SetFrequency(double f);
extern double Freq_From_Register(double fcryst);
extern uint8_t Si570Init(uint8_t i2c_address);
extern uint8_t Si570NewFreq(uint8_t i2c_address);
extern uint8_t Si570FreezeNCO(uint8_t i2c_address);
extern uint8_t Si570UnFreezeNCO(uint8_t i2c_address);
extern uint8_t WriteCmdToSi570(uint8_t i2c_address, uint8_t reg, uint8_t d);
extern uint8_t WriteRegToSi570(uint8_t i2c_address);
extern uint8_t GetRegFromSi570(uint8_t i2c_address);

#endif /* SI570_H_ */
