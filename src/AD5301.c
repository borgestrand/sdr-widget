/*
 * AD5301.c
 *
 *  Created on: 2010-02-21
 *      Author: TF3LJ
 */

#include "AD5301.h"

/*! \brief Write data to the AD5301 DAC
 *
 * \retval I2C status.
 */

uint8_t ad5301(uint8_t i2c_address, uint8_t value)
{
	uint8_t data_out[2] = {0,0};
	uint8_t status;
	twi_package_t packet;

	data_out[0] = (value & 0xf0)>>4;		// Move upper 4 bits into lower 4 bits
	data_out[1] = (value & 0x0f)<<4;		// Move lower 4 bits into upper 4 bits

	xSemaphoreTake( mutexI2C, portMAX_DELAY );

	packet.chip = i2c_address;
	packet.addr = 0x00;
	packet.addr_length = 0;
	packet.buffer = data_out;
	packet.length = 2;

	status = twi_master_write(MOBO_TWI, &packet);

	// Release I2C port
	xSemaphoreGive( mutexI2C );

	return status;
}



