/*
 * PCF8574.c
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#include <stdint.h>

#include "PCF8574.h"
#include "I2C.h"

#if I2C_LCD_PRINT
#include "Mobo_config.h"
#include "taskLCD.h"
#endif

// pcf8574_mobo_data_out contains the current output data on the builtin PCF8574 on the Mobo
volatile uint8_t	pcf8574_mobo_data_out = 0xfa;// Not really necessary, set initial BPF for filter 2.


/*! \brief Write all 8 bits to the PCF8574
 *
 * \retval I2C status.
 */
uint8_t pcf8574_out_byte(uint8_t i2c_address, uint8_t data)
{
	uint8_t status;

	status = twi_write_out(i2c_address, &data,sizeof(data));

	return status;
}


/*! \brief Read all 8 bits from the PCF8574
 *
 * \retval I2C status.
 */
uint8_t pcf8574_in_byte(uint8_t i2c_address, uint8_t *read_byte)
{
	uint8_t status;

	status = twi_read_in(i2c_address, read_byte, sizeof(read_byte));

	return status;
}


/*! \brief Set output bits in the PCF8574 built into to the Mobo 4.3
 *
 * \retval I2C status.
 */
uint8_t pcf8574_mobo_set(uint8_t i2c_address, uint8_t byte)
{
	uint8_t	status;
	// pcf_data_out contains the current output data on the builtin PCF8574 on the Mobo
	pcf8574_mobo_data_out = pcf8574_mobo_data_out | byte;			// Set bits
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
	pcf8574_mobo_data_out = pcf8574_mobo_data_out & ~byte;			// Clear bits
	status = pcf8574_out_byte(i2c_address, pcf8574_mobo_data_out);		// Write out to Mobo PCF8574
	return status;
}

