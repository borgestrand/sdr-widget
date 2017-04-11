/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */

/* Written by Borge Strand-Bergesen 20150701
 * Interaction with WM8805
 *
 * Copyright (C) Borge Strand-Bergesen
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

#ifndef WM8805_H_
#define WM8805_H_

#include <stdint.h>


// Various WM8805 functions are drafted here and later moved somewhere better.....
// Using the WM8805 requires intimate knowledge of the chip and its datasheet. For this
// reason we use a lot of raw hex rather than naming of its internal registers.

#define	WM8805_RESET_START	1
#define WM8805_RESET_END	0
#define WM8805_RESET_PIN	AVR32_PIN_PX10
#define WM8805_CSB_PIN		AVR32_PIN_PX37		// Only applies in HW_GEN_DIN20. FIX: Rather use ext. pull-down for pin 9?
#define WM8805_INT_N_PIN	AVR32_PIN_PX54
#define WM8805_ZERO_PIN		AVR32_PIN_PX15
#define WM8805_DEV_ADR		0x3A 				// 0x3A with pin 9 patched to GND with 10k 00111010
#define WM8805_PLL_NONE		0
#define WM8805_PLL_NORMAL	1					// PLL mode is normal 32-96 and 176.4ksps
#define WM8805_PLL_192		2					// PLL mode is for 192ksps
#define WM8805_PLL_EXP		2					// Experimental PLL mode
#define WM8805_UNLOCK_LIM	2					// Number of poll cycles to determine that an unlock has taken place, in order to start searching
#define WM8805_HICKUP_LIM	6 // 6				// Number of poll cycles to permit hickups in selected audio channel before searching
#define WM8805_PAUSE_LIM	350	// 400 // 200	// Poll cycles to determine that a currently  playing input is silent. NB: Signed 16-bit number!
#define WM8805_SILENCE_LIM	20	//40			// Poll cycles to wait for a mute input to produce audio
#define WM8805_LOCK_LIM		3					// Poll cycles to verify lock

typedef struct wm8805_status {					// Definition of global variable
	uint8_t powered;
	uint8_t muted;
	uint8_t silent;
	uint8_t reliable;
	uint32_t frequency;
	uint8_t pllmode;
} wm8805_status_t;

// Regular polling of WM8805 hardware
void wm8805_poll(void);

// Reset the WM8805 via hardware pin
void wm8805_reset(uint8_t reset_type);

// Start up the WM8805
void wm8805_init(void);

// Turn off wm8805, why can't we just run init again?
void wm8805_sleep(void);

// Select input channel of the WM8805
void wm8805_input(uint8_t input_sel);

// Select PLL setting for the WM8805
void wm8805_pll(uint8_t pll_sel);

// Set up WM8805 CLKOUTDIV so that CLKOUT is in the 22-24MHz range
void wm8805_clkdiv(void);

// Mute the WM8805 output by means of other hardware
void wm8805_mute(void);

// Un-mute the WM8805 output by means of other hardware
void wm8805_unmute(void);

// Write a single byte to WM8805
uint8_t wm8805_write_byte(uint8_t int_adr, uint8_t data);

// Read a single byte from WM8805
uint8_t wm8805_read_byte(uint8_t int_adr);

// Sample rate detection test
uint32_t wm8805_srd(void);
uint32_t wm8805_srd_asm(void);
uint32_t wm8805_srd_asm2(void);		// For test and evaluation


#endif /* WM8805_H_ */
