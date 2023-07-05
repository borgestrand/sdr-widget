/* -*- mode: c; tab-width: 4; c-basic-offset: 4 -*- */
#ifndef _SDR_Widget_H_
#define _SDR_Widget_H_

 /*
 * Additions and Modifications to ATMEL AVR32-SoftwareFramework-AT32UC3 are:
 *
 * Copyright (C) Alex Lee
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "compiler.h"

#define FIRMWARE_VERSION	"V:ALL-006"

/*! \name Peripherals to include at compile time. */
//! @{
#define I2C                 1	// I2C driver
#define AK5394              1   // 24-bit ADC, too much hinges on this one!

#define DEBUG232            1   // Use the UART debug port
#define USB                 1   // Although it may look odd there may be a free standing mode.
#define LED                 1   // Flashy-Blinky lights
// #define SPI                 0   // SPI driver
// #define EXTERN_MEM          0   // Use MMC memory slot

//! @}

// Low Pass Filters for Transmit ----------------------------------------------------------
#define TX_FILTERS			0 // 1 Disabled 20171228	// Enable TX filter selection, including one of the four options below
// Only one of the four below should be selected, if TX_FILTERS is selected
#define	PCF_LPF				0 // 1 Disabled 20171228	// External Port Expander Control of Low Pass filters
#define	PCF_16LPF			0	// External Port Expander Control of 16 Low Pass filters
#define PCF_FILTER_IO		0	// 8x BCD control for LPF switching, switches P1 pins 4-6
#define M0RZF_FILTER_IO		0	// M0RZF 20W amplifier LPF switching, switches P1 pins 4-6






#endif  // _SDR_Widget_H_
