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

#ifndef WM8804_H_
#define WM8804_H_

#include <stdint.h>


// Various WM8804 functions are drafted here and later moved somewhere better.....
// Using the WM8804 requires intimate knowledge of the chip and its datasheet. For this
// reason we use a lot of raw hex rather than naming of its internal registers.

// RXMODFIX Redefine all IO pins

#define	WM8804_RESET_START	1
#define WM8804_RESET_END	0
#define WM8804_RESET_PIN	AVR32_PIN_PX10		// Re-pinned
#define WM8804_CSB_PIN		AVR32_PIN_PX15		// Re-pinned
#define WM8804_INT_N_PIN	AVR32_PIN_PA04		// Re-pinned
#define WM8804_ZERO_PIN		AVR32_PIN_PX54		// Re-pinned, double definition!
#define WM8804_SWIFMODE_PIN	AVR32_PIN_PX54		// RXMODFIX use pull-down, double definition!

// RXMODFIX define GPO-pins here?

#define WM8804_DEV_ADR		0x3A 				// 0x3A with pin 9 patched to GND with 10k 00111010 // Ported
#define WM8804_PLL_NONE		0
#define WM8804_PLL_NORMAL	1					// PLL mode is normal 32-96 and 176.4ksps
#define WM8804_PLL_192		2					// PLL mode is for 192ksps
#define WM8804_PLL_EXP		2					// Experimental PLL mode
#define WM8804_PLL_TOGGLE	3					// Switch to opposite PLL mode
#define WM8804_UNLOCK_LIM	2					// Number of poll cycles to determine that an unlock has taken place, in order to start searching
#define WM8804_HICKUP_LIM	6 // 6				// Number of poll cycles to permit hickups in selected audio channel before searching
#define WM8804_PAUSE_LIM	350	// 400 // 200	// Poll cycles to determine that a currently  playing input is silent. NB: Signed 16-bit number!
#define WM8804_SILENCE_LIM	20	//40			// Poll cycles to wait for a mute input to produce audio
#define WM8804_LOCK_LIM		3					// Poll cycles to verify lock
#define WM8804_CLK_FAILURE	0					// Failed setting of internal clock division
#define WM8804_CLK_SUCCESS	1					// Successful setting of internal clock division

typedef struct spdif_rx_status {				// Definition of global variable
	uint8_t powered;
	uint8_t muted;
	uint8_t silent;
	uint8_t reliable;
	uint32_t frequency;
	uint8_t pllmode;
	uint8_t buffered;
} spdif_rx_status_t;

// Regular polling of WM8804 hardware
void wm8804_poll(void);

// Reset the WM8804 via hardware pin
void wm8804_reset(uint8_t reset_type);

// Start up the WM8804
void wm8804_init(void);

// Start up the WM8804 config task
extern void wm8804_task_init(void);

// The config task itself
extern void wm8804_task(void *pvParameters);

// Turn off wm8804, why can't we just run init again?
void wm8804_sleep(void);


// Scan inputs, start of new scanning algorithm. Doesn't detect silence
void wm8804_scannew(uint8_t *channel, uint32_t *freq, uint8_t mode);

// Select input channel of the WM8804
void wm8804_input(uint8_t input_sel);
uint32_t wm8804_inputnew(uint8_t input_sel);

// Select PLL setting for the WM8804
void wm8804_pll(void);
void wm8804_pllnew(uint8_t pll_sel);			// For manual control

// Set up WM8804 CLKOUTDIV so that CLKOUT is in the 22-24MHz range
void wm8804_clkdiv(void);
uint8_t wm8804_clkdivnew(uint32_t freq);

// Mute the WM8804 output by means of other hardware
void wm8804_mute(void);

// Un-mute the WM8804 output by means of other hardware
void wm8804_unmute(void);

// Write a single byte to WM8804
uint8_t wm8804_write_byte(uint8_t int_adr, uint8_t data);

// Read a single byte from WM8804
uint8_t wm8804_read_byte(uint8_t int_adr);

// Sample rate detection test
uint32_t wm8804_srd(void);
uint32_t wm8804_srd_asm(void);
uint32_t wm8804_srd_asm2(void);		// For test and evaluation


#endif /* WM8804_H_ */
