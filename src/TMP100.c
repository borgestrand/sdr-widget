/*
 * TMP100.c
 *
 *  Created on: 2010-02-21
 *      Author: TF3LJ
 */

#include "TMP100.h"

// A global containing the last measured value read from the TMP100 temperature sensor
int16_t	tmp100_data;

/*! \brief Initialise the TMP100 into 12 bit (high precision) mode
 *
 * \retval I2C status.
 */
uint8_t tmp100_init(uint8_t i2c_address)
{
	uint8_t status;
	twi_package_t packet;
	uint8_t set_hidef[] = {0x01,0xe0};			// Set the TMP100 into 12 bit mode
	uint8_t set_read  = 0x00;					// Set the TMP100 into Read mode

	// Wait for I2C port to become free
	xSemaphoreTake( mutexI2C, portMAX_DELAY );

	packet.chip = i2c_address;
	packet.addr = 0;
	packet.addr_length = 0;
	packet.buffer = set_hidef;
	packet.length = 2;

	twi_master_write(MOBO_TWI, &packet);

	packet.buffer = &set_read;
	packet.length = 1;
	status = twi_master_write(MOBO_TWI, &packet);

	// Release I2C port
	xSemaphoreGive( mutexI2C );

	return status;
}

/*! \brief Read temperature from TMP100 device
 *
 * Returned temperature reading can be converted to degrees C, by using the formula:
 * temp = 128.0 / 32768 * tmp100().i;
 *
 * \retval I2C status.
 */
uint8_t tmp100_read(uint8_t i2c_address)
{
	uint8_t status;
	twi_package_t packet;

	xSemaphoreTake( mutexI2C, portMAX_DELAY );

	packet.chip = i2c_address;
	packet.addr = 0x00;
	packet.addr_length = 0;
	packet.buffer = &tmp100_data;
	packet.length = 2;

	status = twi_master_read(MOBO_TWI, &packet);

	// Release I2C port
	xSemaphoreGive( mutexI2C );

	return status;
}


