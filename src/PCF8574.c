/*
 * PCF8574.c
 *
 *  Created on: 2010-02-20
 *      Author: TF3LJ
 */

#include "PCF8574.h"


// pcf_data_out contains the current output data on the builtin PCF8574 on the Mobo
uint8_t	pcf8574_mobo_data_out = 0xff;


/*! \brief Set output bits in the PCF8574 built into to the Mobo 4.3
 *
 * \retval I2C status.
 */
uint8_t pcf8574_mobo_set(uint8_t i2c_address, uint8_t byte)
{
	uint8_t	status;
	// pcf_data_out contains the current output data on the builtin PCF8574 on the Mobo
	pcf8574_mobo_data_out = pcf8574_mobo_data_out | byte;				// Set bits
	status = pcf8574_out_byte(i2c_address, pcf8574_mobo_data_out);		// Write out to Mobo PCF8574
	return status;
}

/*! \brief Clear output bits in the PCF8574 built into to the Mobo 4.3
 *
 * \retval I2C status.
 */
uint8_t pcf8574_mobo_clear(uint8_t i2c_address, uint8_t byte)
{
	uint8_t	status;
	// pcf_data_out contains the current output data on the builtin PCF8574 on the Mobo
	pcf8574_mobo_data_out = pcf8574_mobo_data_out & ~byte;				// Clear bits
	status = pcf8574_out_byte(i2c_address, pcf8574_mobo_data_out);		// Write out to Mobo PCF8574
	return status;
}

/*! \brief Write all 8 bits to the PCF8574
 *
 * \retval I2C status.
 */
uint8_t pcf8574_out_byte(uint8_t i2c_address, uint8_t data)
{
	uint8_t	status;
	twi_package_t packet;

	// Wait for I2C port to become free
	xSemaphoreTake( mutexI2C, portMAX_DELAY );

	packet.chip = i2c_address;
	packet.addr = 0x00;
	packet.addr_length = 0;
	packet.buffer = &data;
	packet.length = 1;

	status = twi_master_write(MOBO_TWI, &packet);

	// Release I2C port
	xSemaphoreGive( mutexI2C );

	return status;
}

/*! \brief Read all 8 input bits from the PCF8574
 *
 * \retval I2C status.
 */
uint8_t pcf8574_in_byte(uint8_t i2c_address, uint8_t *data_to_return)
{
	uint8_t status;
	twi_package_t packet;

	// Wait for I2C port to become free
	xSemaphoreTake( mutexI2C, portMAX_DELAY );

	packet.chip = i2c_address;
	packet.addr = 0x00;
	packet.addr_length = 0;
	packet.buffer = data_to_return;
	packet.length = 1;

	status = twi_master_read(MOBO_TWI, &packet);

	// Release I2C port
	xSemaphoreGive( mutexI2C );

	return status;
}
