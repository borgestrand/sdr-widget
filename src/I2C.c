/*
 * I2C.c
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

/*
 * Atmel specific headers.
 */
//#include "print_funcs.h"	// print_dbg(str)

/*
 * Widget TASK headers
 */
#include <stdint.h>

#include "I2C.h"

portBASE_TYPE xStatus;
// xSemaphoreHandle mutexI2C; // Switching to mutex I2C_busy_semphr


void twi_init(void)
{
	#ifdef MOBO_TWIM1
	static const gpio_map_t MOBO_TWI_GPIO_MAP =
	{
			{TWIM1_SCL_PIN, TWIM1_SCL_FUNCTION},
			{TWIM1_SDA_PIN, TWIM1_SDA_FUNCTION}
	};
	#endif
	#ifdef MOBO_TWIM0
	static const gpio_map_t MOBO_TWI_GPIO_MAP =
	{
			{TWIM0_SCL_PIN, TWIM0_SCL_FUNCTION},
			{TWIM0_SDA_PIN, TWIM0_SDA_FUNCTION}
	};
	#endif

	static const twi_options_t MOBO_TWI_OPTIONS =
	{
			.pba_hz = FOSC0,
			.speed  = MOBO_TWI_SPEED,
			.chip   = 0,
	};

	// Assign I/O pins to TWI
	gpio_enable_module(MOBO_TWI_GPIO_MAP,
			sizeof(MOBO_TWI_GPIO_MAP) / sizeof(MOBO_TWI_GPIO_MAP[0]));

	// Initialize as master.
	// twi_master_init has been modified in twim.c, to support passing of IRQ vector:
	// int twi_master_init(volatile avr32_twim_t *twi, const twi_options_t *opt, const unsigned irq)
	// irq is passed on in twim.c, modified line 186:
	// INTC_register_interrupt( &twi_master_interrupt_handler, irq, AVR32_INTC_INT1);
	twi_master_init(MOBO_TWI, &MOBO_TWI_OPTIONS, MOBO_TWI_IRQ);
}

uint8_t twi_write_out(uint8_t i2c_address, uint8_t *payload, uint8_t size)
{
	uint8_t	status;

	// Wait for I2C port to become free
	// xSemaphoreTake( mutexI2C, portMAX_DELAY ); // Switching to semmutex I2C_busy_semphr

	twi_package_t packet =
	{
		.chip = i2c_address,
		.addr = 0x00,
		.addr_length = 0,
		.buffer = (void*)payload,
		.length = size
	};
	// Returns TWI_SUCCESS (0) on successful write
	status=twi_master_write(MOBO_TWI, &packet);

	// Release I2C port
	// xSemaphoreGive( mutexI2C ); // Switching to mutex  I2C_busy_semphr

	return status;
}

uint8_t twi_read_in(uint8_t i2c_address, uint8_t *data_to_return, uint8_t size)
{
	uint8_t status;

	// Wait for I2C port to become free
//	xSemaphoreTake( mutexI2C, portMAX_DELAY ); // Switching to mutex I2C_busy_semphr

	twi_package_t packet =
	{
		.chip = i2c_address,
		.addr = 0,
		.addr_length = 0,
		.buffer = data_to_return,
		.length = size
	};

	// Returns TWI_SUCCESS (0) on successful read
	status = twi_master_read(MOBO_TWI, &packet);

	// Release I2C port
//	xSemaphoreGive( mutexI2C );  // Switching to mutex I2C_busy_semphr

	return status;
}


