/*
 * AD7991.c
 *
 *  Created on: 2010-02-21
 *      Author: TF3LJ
 */

#include "AD7991.h"

uint16_t	ad7991_adc[4];						// Last measured values read from the AD7991 ADC
												// all values adjusted for a full scale 16 bit unsigned int

// Looks like this is not needed.
/*! \brief Setup AD7991 to do interesting stuff
 *
 * \retval I2C status.
 */
/*void ad7991_setup(uint8_t i2c_address)
{
	uint8_t data_out = 0xf0;			// Setup Reg.  0xf0 is default setup
										// meaning that all 4 A/Ds are polled,
										// Vref = Vdd etc...
	uint8_t status;
	twi_package_t packet;

	xSemaphoreTake( mutexI2C, portMAX_DELAY );

	packet.chip = i2c_address;
	packet.addr = 0x00;
	packet.addr_length = 0;
	packet.buffer = data_out;
	packet.length = 1;

	status = twi_master_write(MOBO_TWI, &packet);

	// Release I2C port
	xSemaphoreGive( mutexI2C );

	return status;
}
*/

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
	twi_package_t packet;

	xSemaphoreTake( mutexI2C, portMAX_DELAY );

	packet.chip = i2c_address;
	packet.addr = 0x00;
	packet.addr_length = 0;
	packet.buffer = raw_read;
	packet.length = 8;

	status = twi_master_read(MOBO_TWI, &packet);

	// Release I2C port
	xSemaphoreGive( mutexI2C );

	// Write left adjusted into global var uint16_t	ad7991_adc[4]

	for (i = 0; i<8;i+=2)
	{
		if ((raw_read[i]>>4) < 4)			// If data not garbled
			ad7991_adc[raw_read[i]>>4] = 0x100*(((raw_read[i] & 0x0f)<<4)
				+((raw_read[i+1] & 0xf0)>>4)) + ((raw_read[i+1] & 0x0f)<<4);
	}

	return status;
}

