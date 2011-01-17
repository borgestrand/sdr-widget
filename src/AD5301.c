/*
 * AD5301.c
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#include <stdint.h>

#include "AD5301.h"
#include "I2C.h"

/*! \brief Write data to the AD5301 DAC
 *
 * \retval I2C status.
 */

uint8_t ad5301(uint8_t i2c_address, uint8_t value)
{
	uint8_t data_out[2] = {0,0};
	uint8_t status;

	data_out[0] = (value & 0xf0)>>4;		// Move upper 4 bits into lower 4 bits
	data_out[1] = (value & 0x0f)<<4;		// Move lower 4 bits into upper 4 bits

	status = twi_write_out(i2c_address, data_out,sizeof(data_out));

	return status;
}



