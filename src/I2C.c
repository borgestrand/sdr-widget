/*
 * I2C.c
 *
 *  Created on: 2010-02-27
 *      Author: TF3LJ
 */

/*
 * Atmel specific headers.
 */
//#include "print_funcs.h"	// print_dbg(str)

/*
 * Widget TASK headers
 */
#include "I2C.h"

portBASE_TYPE xStatus;
xSemaphoreHandle mutexI2C;


void twi_init(void)
{
	#ifdef MOBO_TWIM1
	const gpio_map_t MOBO_TWI_GPIO_MAP =
	{
			{TWIM1_SCL_PIN, TWIM1_SCL_FUNCTION},
			{TWIM1_SDA_PIN, TWIM1_SDA_FUNCTION}
	};
	#endif
	#ifdef MOBO_TWIM0
	const gpio_map_t MOBO_TWI_GPIO_MAP =
	{
			{TWIM0_SCL_PIN, TWIM0_SCL_FUNCTION},
			{TWIM0_SDA_PIN, TWIM0_SDA_FUNCTION}
	};
	#endif

	const twi_options_t MOBO_TWI_OPTIONS =
	{
			.pba_hz = FOSC0,
			.speed  = MOBO_TWI_SPEED,
			.chip   = 0x00
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

	mutexI2C = xSemaphoreCreateMutex();

}
