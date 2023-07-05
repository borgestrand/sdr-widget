/*
 * I2C.h
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef __TASKI2C_H__
#define __TASKI2C_H__

/*
 * Widget headers
 */
#include "board.h"

/*
 *  freeRTOS headers
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gpio.h"
//#include "FreeRTOS.h"
#include "semphr.h"

#include "twim_patched.h"

//#define MOBO_TWI_SPEED 	400000 // Measures as 470kHz
#define MOBO_TWI_SPEED 	660000	// Measures as 387kHz
//#define MOBO_TWI_SPEED 	670000	// Measures as 427kHz


#define MOBO_TWIM1	// Define as either TWIM0 or TWIM1

#ifdef MOBO_TWIM0
#define MOBO_TWI 		TWIM0			// TWI (I2C) Port 0
#define MOBO_TWI_IRQ 	AVR32_TWIM0_IRQ // Used by tweaked twim.c, twi_master_init()
#endif
#ifdef MOBO_TWIM1
#define MOBO_TWI 		TWIM1			// TWI (I2C) Port 1
#define MOBO_TWI_IRQ 	AVR32_TWIM1_IRQ // Used by tweaked twim.c, twi_master_init()
#endif

// extern xSemaphoreHandle mutexI2C;

extern void twi_init(void);
extern uint8_t twi_write_out(uint8_t i2c_address, uint8_t *payload, uint8_t size);
extern uint8_t twi_read_in(uint8_t i2c_address, uint8_t *data_to_return, uint8_t size);


#endif
