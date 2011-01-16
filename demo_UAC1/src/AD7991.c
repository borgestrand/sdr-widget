/*
 * AD7991.c
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#include <stdint.h>

#include "AD7991.h"
#include "I2C.h"

uint16_t	ad7991_adc[4];						// Last measured values read from the AD7991 ADC
												// all values adjusted for a full scale 16 bit unsigned int

// Looks like this is not needed.
/*! \brief Setup AD7991 to do interesting stuff
 *
 * \retval I2C status.
 */
uint8_t ad7991_setup(uint8_t i2c_address)
{
	uint8_t data_out = 0xf0;			// Setup Reg.  0xf0 is default setup
										// meaning that all 4 A/Ds are polled,
										// Vref = Vdd etc...
	uint8_t status;

	status = twi_write_out(i2c_address, &data_out,sizeof(data_out));

	return status;
}


/*! \brief Poll the AD7991 4 x ADC chip
 *
 *	This function reads all four A/D inputs and makes the data available in four
 *  global variables, ad7991_adc[4]
 *
 * \retval I2C status.
 */
uint8_t ad7991_poll(uint8_t i2c_address)
{
	uint8_t i, status;
	uint8_t raw_read[8];

	status = twi_read_in(i2c_address, raw_read, sizeof(raw_read));

	// Write left adjusted into global var uint16_t	ad7991_adc[4]
	for (i = 0; i<8;i+=2)
	{
		if ((raw_read[i]>>4) < 4)			// If data not garbled
			ad7991_adc[raw_read[i]>>4] = 0x100*(((raw_read[i] & 0x0f)<<4)
				+((raw_read[i+1] & 0xf0)>>4)) + ((raw_read[i+1] & 0x0f)<<4);
	}

	return status;
}

