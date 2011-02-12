/*
 * I2C.h
 *
 *  Created on: 2010-02-27
 *      Author: TF3LJ
 */

#ifndef __TASKI2C_H__
#define __TASKI2C_H__

#include <stdint.h>

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
#include "twim.h"

//#define MOBO_TWI_SPEED 	400000
#define MOBO_TWI_SPEED 	100000

#define MOBO_TWIM1	// Define as either TWIM0 or TWIM1

#ifdef MOBO_TWIM0
#define MOBO_TWI 		TWIM0			// TWI (I2C) Port 0
#define MOBO_TWI_IRQ 	AVR32_TWIM0_IRQ // Used by tweaked twim.c, twi_master_init()
#endif
#ifdef MOBO_TWIM1
#define MOBO_TWI 		TWIM1			// TWI (I2C) Port 1
#define MOBO_TWI_IRQ 	AVR32_TWIM1_IRQ // Used by tweaked twim.c, twi_master_init()
#endif

extern xSemaphoreHandle mutexI2C;

extern void twi_init(void);

#endif
