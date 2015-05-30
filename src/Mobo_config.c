/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * Mobo_config.c
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#include "Mobo_config.h"
#include "features.h"

// To compile sample rate detector we need low-level hardware access
#include "gpio.h"
#include <avr32/io.h>
#include "compiler.h"

// To access global input source variable
#include "device_audio_task.h"

// To access SPK_BUFFER_SIZE and clear audio buffer
#include "taskAK5394A.h"
#include "usb_specific_request.h"

/*
#include "rotary_encoder.h"
#include "AD7991.h"
#include "AD5301.h"
#include "Si570.h"
#include "PCF8574.h"
#include "TMP100.h"
*/

#if defined(HW_GEN_DIN10)

// Various WM8805 functions are drafted here and later moved somewhere better.....
// Using the WM8805 requires intimate knowledge of the chip and its datasheet. For this
// reason we use a lot of raw hex rather than naming of its internal registers.

// Hardware reset over GPIO pin. Consider the Power Up Configuration section of the datasheet!
void wm8805_reset(uint8_t reset_type) {
	if (reset_type == WM8805_RESET_START)
		gpio_clr_gpio_pin(WM8805_RESET_PIN);			// Clear reset pin WM8807 active low reset
	else
		gpio_set_gpio_pin(WM8805_RESET_PIN);			// Set reset pin WM8807 active low reset
}

// Start up the WM8805
void wm8805_init(void) {



	wm8805_write_byte(0x08, 0x70);	// 7:0 CLK2, 6:1 auto error handling disable, 5:1 zeros@error, 4:1 CLKOUT enable, 3:0 CLK1 out, 2-0:000 RX0

	wm8805_write_byte(0x1C, 0xCE);	// 7:1 I2S alive, 6:1 master, 5:0 normal pol, 4:0 normal, 3-2:11 or 10 24 bit, 1-0:10 I2S ? CE or CA ?

	wm8805_write_byte(0x1D, 0xC0);	// Change 6:1, disable data truncation, run on 24 bit I2S

	wm8805_write_byte(0x17, 0x00);	// 7:4 GPO1=INT_N (=SPIO_00, PX54), 3:0 GPO0=INT_N, that pin has 10kpull-down

//	wm8805_write_byte(0x1A, 0xC0);	// 7:4 GPO7=ZEROFLAG (=SPIO_04, PX15), 3:0 GPO6=INT_N, that pin is grounded SPDIF in via write to 0x1D:5
//	wm8805_write_byte(0x1A, 0x70);	// 7:4 GPO7=UNLOCK (=SPIO_04, PX15), 3:0 GPO6=INT_N, that pin is grounded SPDIF in via write to 0x1D:5
	wm8805_write_byte(0x1A, 0x40);	// 7:4 GPO7=TRANS_ERR (=SPIO_04, PX15), 3:0 GPO6=INT_N, that pin is grounded SPDIF in via write to 0x1D:5

	wm8805_write_byte(0x0A, 0b11100100);	// REC_FREQ:mask (broken in wm!), DEEMPH:ignored, CPY:ignored, NON_AUDIO:active
											// TRANS_ERR:active, CSUD:ignored, INVALID:active, UNLOCK:active

	wm8805_read_byte(0x0B);			// Clear interrupts

	wm8805_write_byte(0x1E, 0x1B);	// Power down 7-6:0, 5:0 OUT, 4:1 _IF, 3:1 _OSC, 2:0 TX, 1:1 _RX, 0:1 _PLL,
}

// Turn off wm8805, why can't we just run init again?
void wm8805_sleep(void) {
	wm8805_write_byte(0x1E, 0x1B);	// Power down 7-6:0, 5:0 OUT, 4:1 _IF, 3:1 _OSC, 2:0 TX, 1:1 _RX, 0:1 _PLL,
}

// Select input channel of the WM8805
void wm8805_input(uint8_t input_sel) {
/*
 * Mute ???
 * Disable RX, select channel, enable RX
 * Wait for lock and report success/failure?
 * Unmute ???
 */

// FIX: if input is USB, do some major shutting down, if it aint, reinit the WM8805!

	wm8805_write_byte(0x1E, 0x06);		// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:1 _RX, 0:0 PLL,

	if (input_sel == MOBO_SRC_TOSLINK)
		wm8805_write_byte(0x08, 0x34);	// 7:0 CLK2, 6:0 auto error handling enable, 5:1 zeros@error, 4:1 CLKOUT enable, 3:0 CLK1 out, 2-0:4 RX4
 	else if (input_sel == MOBO_SRC_SPDIF)
		wm8805_write_byte(0x08, 0x35);	// 7:0 CLK2, 6:0 auto error handling enable, 5:1 zeros@error, 4:1 CLKOUT enable, 3:0 CLK1 out, 2-0:5 RX5

	wm8805_write_byte(0x1E, 0x04);		// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:0 RX, 0:0 PLL,
}

// Select input channel of the WM8805
void wm8805_pll(uint8_t pll_sel) {
/*
 * Mute ???
 * Disable RX, set up PLL, enable RX
 * Wait for lock and report success/failure?
 * Unmute ???
 */

	wm8805_write_byte(0x1E, 0x06);		// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:1 _RX, 0:0 PLL,

	// Default PLL setup for 44.1, 48, 88.2, 96, 176.4
	if (pll_sel == WM8805_PLL_NORMAL) {
		wm8805_write_byte(0x03, 0x21);	// PLL_K[7:0] 21
		wm8805_write_byte(0x04, 0xFD);	// PLL_K[15:8] FD
		wm8805_write_byte(0x05, 0x36);	// 7:0 , 6:0, 5-0:PLL_K[21:16] 36
		wm8805_write_byte(0x06, 0x07);	// 7:0 , 6:0 , 5:0 , 4:0 Prescale/1 , 3-2:PLL_N[3:0] 7
	}

	// Special PLL setup for 192
	else if (pll_sel == WM8805_PLL_192) {	// PLL setting 8.192
		wm8805_write_byte(0x03, 0xBA);	// PLL_K[7:0] BA
		wm8805_write_byte(0x04, 0x49);	// PLL_K[15:8] 49
		wm8805_write_byte(0x05, 0x0C);	// 7:0,  6:0, 5-0:PLL_K[21:16] 0C
		wm8805_write_byte(0x06, 0x08);	// 7: , 6: , 5: , 4: , 3-2:PLL_N[3:0] 8
	}

/* // Bad news: unified PLL setting doesn't work!
	else if (pll_sel == WM8805_PLL_EXP) { 	// Experimental PLL setting 8.0247 failed 192, 8.1 failed 176 and 192. Forget it!
		wm8805_write_byte(0x03, 0x66);	// PLL_K[7:0]
		wm8805_write_byte(0x04, 0x66);	// PLL_K[15:8]
		wm8805_write_byte(0x05, 0x06);	// 7:0,  6:0, 5-0:PLL_K[21:16]
		wm8805_write_byte(0x06, 0x08);	// 7: , 6: , 5: , 4: , 3-2:PLL_N[3:0] 8
	}
*/
	wm8805_write_byte(0x1E, 0x04);		// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:0 RX, 0:0 PLL,
}

// Set up WM8805 CLKOUTDIV so that CLKOUT is in the 22-24MHz range
void wm8805_clkdiv(void) {
	uint8_t temp;
	temp = wm8805_read_byte(0x0C);		// Read SPDSTAT
	temp = temp & 0x30;					// Consider bits 5-4

	if ( (temp == 0x20) || (temp == 0x30) )	// 44.1, 48, or 32
		wm8805_write_byte(0x07, 0x0C);	// 7:0 , 6:0, 5-4:MCLK=512fs , 3:1 MCLKDIV=1 , 2:1 FRACEN , 1-0:0
	else if (temp == 0x10)				// 88.2 or 96
		wm8805_write_byte(0x07, 0x1C);	// 7:0 , 6:0, 5-4:MCLK=256fs , 3:1 MCLKDIV=1 , 2:1 FRACEN , 1-0:0
	else								// 176.4 or 192
		wm8805_write_byte(0x07, 0x2C);	// 7:0 , 6:0, 5-4:MCLK=128fs , 3:1 MCLKDIV=1 , 2:1 FRACEN , 1-0:0
}

// Mute the WM8805 output by means of other hardware
void wm8805_mute(void) {
	int i;

	for (i = 0; i < SPK_BUFFER_SIZE; i++) {		// Clear USB subsystem's buffer in order to mute I2S
		spk_buffer_0[i] = 0;
		spk_buffer_1[i] = 0;
	}

	if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio)
    	mobo_xo_select(current_freq.frequency, MOBO_SRC_UAC1);	// Mute WM8805 by relying on USB subsystem's presumably muted output
	else
    	mobo_xo_select(current_freq.frequency, MOBO_SRC_UAC2);	// Mute WM8805 by relying on USB subsystem's presumably muted output
	print_dbg_char('l');						// Not-loud!
}

// Un-mute the WM8805 output by means of other hardware
void wm8805_unmute(void) {
	U32 wm8805_freq;
	wm8805_freq = mobo_srd();
	mobo_led_select(wm8805_freq, input_select);	// Indicate present sample rate
	mobo_xo_select(wm8805_freq, input_select);	// Unmute WM8805
	print_dbg_char('L');						// Loud!
}

// Write a single byte to WM8805
uint8_t wm8805_write_byte(uint8_t int_adr, uint8_t int_data) {
    uint8_t dev_data[2];
    uint8_t status;

	dev_data[0] = int_adr;
	dev_data[1] = int_data;

	status = twi_write_out(WM8805_DEV_ADR, dev_data, 2);
	return status;
}

// Read a single byte from WM8805
uint8_t wm8805_read_byte(uint8_t int_adr) {
	uint8_t dev_data[1];

	dev_data[0] = int_adr;
	twi_write_out(WM8805_DEV_ADR, dev_data, 1);
	twi_read_in(WM8805_DEV_ADR, dev_data, 1);
	return dev_data[0];
}

/* Todo list for DIN10
 * + Categorize sample rate detector with proper signal generator, +-2%, reduce timeout constant everywhere
 * - Determine correct GPIO pin for SRD, recompile for asm constants
 * + Test USB music playback with SRD running continuously
 * - Test codebase with mkII hardware
 * + Get hardware capable of generating all SPDIF sample rates
 * - Make state machine for WM8805 sample rate detection
 * - Make state machine for source selection
 * - Figure out ADC interface
 * - Make silence detector (use 1024 silent block detector in WM?)
 */

// Sample rate detection test
U32 mobo_srd(void) {
	int16_t timeout;

	// Using #define TIMEOUT_LIM 150 doesn't seem to work inside asm(), so hardcode constant 150 everywhere!
	// see srd_test.c and srd_test.lst

	// Determining speed at TP16 / DAC_0P / PA04 for now. Recompile prototype c to change io pin!
	// Test is done for up to 1 half period, then 2 full periods

	gpio_enable_gpio_pin(AVR32_PIN_PA04);	// Enable GPIO pin, not special IO (also for input). Needed?

	asm volatile(
		"ssrf	16				\n\t"	// Disable global interrupt
		"mov	%0, 	150		\n\t"	// Load timeout
		"mov	r9,		-61440	\n\t"	// Immediate load, set up pointer to PA04, (0xFFFF1000) recompile C for other IO pin, do once

		// If bit is 0, branch to loop while 0. If bit was 1, continue to loop while 1
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	L3				\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)

		// Wait while bit is 1, then count two half periods
		"L0:					\n\t"	// Loop while PA04 is 1
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	L0_done			\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L0				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L0_done:				\n\t"

		"mov	%0, 	150		\n\t"	// Restart countdon for actual timing of below half-cycles

		"L1:					\n\t"	// Loop while PA04 is 0
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	L1_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L1				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L1_done:				\n\t"

		"L2:					\n\t"	// Loop while PBA04 is 1
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	L2_done			\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L2				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L2_done:				\n\t"
		"rjmp	RETURN__		\n\t"

		// Wait while bit is 0, then count two half periods
		"L3:					\n\t"	// Loop while PA04 is 0
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	L3_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L3				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L3_done:				\n\t"

		"mov	%0, 	150		\n\t"	// Restart countdon for actual timing of below half-cycles

		"L4:					\n\t"	// Loop while PBA04 is 1
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	L4_done			\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L4				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L4_done:				\n\t"

		"L5:					\n\t"	// Loop while PA04 is 0
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	L5_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L5				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L5_done:				\n\t"
		"rjmp	RETURN__		\n\t"


		"COUNTD:				\n\t"	// Countdown reached, %0 is 0

		"RETURN__:				\n\t"
		"csrf	16				\n\t"	// Enable global interrupt
		:	"=r" (timeout)				// One output register
		:								// No input registers
		:	"r8", "r9"					// Clobber registers, pushed/popped unless assigned by GCC as temps
	);

	timeout = 150 - timeout;

	// It looks like we have approx. With 66MHz CPU clock it looks like 15 ticks per detection loop.
	// Results with Scope at +-1% and two half-periods:
	//  32.0 0x81-0x84	// <= 132, this dictates the timeout at 150
	//  44.1 0x5D-0x60
	//  48.0 0x56-0x58
	//	88.2 0x2E-0x2F
	//  96.0 0x29-0x2B
	// 176.4 0x16-0x17
	// 192.0 0x14-0x15	// That's maybe a bit tight between 0x14, 0x15, 0x16 and 0x17... Equal probablility 15/16@184.2kHz..

	#define FLIM_32_LOW		0x81
	#define FLIM_32_HIGH	0x84
	#define FLIM_44_LOW		0x5D
	#define FLIM_44_HIGH	0x60
	#define FLIM_48_LOW		0x56
	#define FLIM_48_HIGH	0x58
	#define FLIM_88_LOW		0x2E
	#define FLIM_88_HIGH	0x2F
	#define FLIM_96_LOW		0x29
	#define FLIM_96_HIGH	0x2B
	#define FLIM_176_LOW	0x16
	#define FLIM_176_HIGH	0x17
	#define FLIM_192_LOW	0x14
	#define FLIM_192_HIGH	0x15

	if ( (timeout >= FLIM_32_LOW) && (timeout <= FLIM_32_HIGH) )
		return FREQ_32;
	if ( (timeout >= FLIM_44_LOW) && (timeout <= FLIM_44_HIGH) )
		return FREQ_44;
	if ( (timeout >= FLIM_48_LOW) && (timeout <= FLIM_48_HIGH) )
		return FREQ_48;
	if ( (timeout >= FLIM_88_LOW) && (timeout <= FLIM_88_HIGH) )
		return FREQ_88;
	if ( (timeout >= FLIM_96_LOW) && (timeout <= FLIM_96_HIGH) )
		return FREQ_96;
	if ( (timeout >= FLIM_176_LOW) && (timeout <= FLIM_176_HIGH) )
		return FREQ_176;
	if ( (timeout >= FLIM_192_LOW) && (timeout <= FLIM_192_HIGH) )
		return FREQ_192;

	return FREQ_TIMEOUT;	// Every uncertainty treated as timeout...
}


// Audio Widget HW_GEN_DIN10 LED control
void mobo_led(uint8_t fled2, uint8_t fled1, uint8_t fled0) {
	// red:1, green:2, blue:4
	// fled2 is towards center of box, fled0 towards right-hand edge of front view

	if (fled0 & FLED_RED)
		gpio_clr_gpio_pin(AVR32_PIN_PA17); 	// FLED0_R
	else
		gpio_set_gpio_pin(AVR32_PIN_PA17); 	// FLED0_R
	if (fled0 & FLED_GREEN)
		gpio_clr_gpio_pin(AVR32_PIN_PA24); 	// FLED0_G
	else
		gpio_set_gpio_pin(AVR32_PIN_PA24); 	// FLED0_G
	if (fled0 & FLED_BLUE)
		gpio_clr_gpio_pin(AVR32_PIN_PA18); 	// FLED0_B
	else
		gpio_set_gpio_pin(AVR32_PIN_PA18); 	// FLED0_B

	if (fled1 & FLED_RED)
		gpio_clr_gpio_pin(AVR32_PIN_PA23); 	// FLED1_R
	else
		gpio_set_gpio_pin(AVR32_PIN_PA23); 	// FLED1_R
	if (fled1 & FLED_GREEN)
		gpio_clr_gpio_pin(AVR32_PIN_PC01); 	// FLED1_G
	else
		gpio_set_gpio_pin(AVR32_PIN_PC01); 	// FLED1_G
	if (fled1 & FLED_BLUE)
		gpio_clr_gpio_pin(AVR32_PIN_PA21); 	// FLED1_B
	else
		gpio_set_gpio_pin(AVR32_PIN_PA21); 	// FLED1_B

	if (fled2 & FLED_RED)
		gpio_clr_gpio_pin(AVR32_PIN_PX29); 	// FLED2_R
	else
		gpio_set_gpio_pin(AVR32_PIN_PX29); 	// FLED2_R
	if (fled2 & FLED_GREEN)
		gpio_clr_gpio_pin(AVR32_PIN_PX32); 	// FLED2_G
	else
		gpio_set_gpio_pin(AVR32_PIN_PX32); 	// FLED2_G
	if (fled2 & FLED_BLUE)
		gpio_clr_gpio_pin(AVR32_PIN_PC00); 	// FLED2_B
	else
		gpio_set_gpio_pin(AVR32_PIN_PC00); 	// FLED2_B
}
#endif

/*! \brief Audio Widget select oscillator
 *
 * \retval none
 */
void mobo_xo_select(U32 frequency, uint8_t source) {

// XO control and SPI muxing on ab1x hardware generation
#if defined(HW_GEN_AB1X)
	switch (frequency) {
		case FREQ_44:
			if (FEATURE_BOARD_USBI2S)
				gpio_clr_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 22.5792MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_clr_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL0);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL1);
		break;
		case FREQ_48:
			if (FEATURE_BOARD_USBI2S)
				gpio_set_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 24.576MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_set_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL0);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL1);
		break;
		case FREQ_88:
			if (FEATURE_BOARD_USBI2S)
				gpio_clr_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 22.5792MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_clr_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL1);
			gpio_set_gpio_pin(SAMPLEFREQ_VAL0);
		break;
		case FREQ_96:
			if (FEATURE_BOARD_USBI2S)
				gpio_set_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 24.576MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_set_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL1);
			gpio_set_gpio_pin(SAMPLEFREQ_VAL0);
		break;
		case FREQ_176:
			if (FEATURE_BOARD_USBI2S)
				gpio_clr_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 22.5792MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_clr_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL0);
			gpio_set_gpio_pin(SAMPLEFREQ_VAL1);
		break;
		case FREQ_192:
			if (FEATURE_BOARD_USBI2S)
				gpio_set_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 24.576MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_set_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL0);
			gpio_set_gpio_pin(SAMPLEFREQ_VAL1);
		break;
	}

// XO control and I2S muxing on Digital Input 1.0 generation
#elif defined(HW_GEN_DIN10)
	// FIX: correlate with mode currently selected by user or auto, that's a global variable!
	if ( (source == MOBO_SRC_UAC1) || (source == MOBO_SRC_UAC2) || (source == MOBO_SRC_NONE) ) {
		gpio_clr_gpio_pin(AVR32_PIN_PX44); 			// SEL_USBN_RXP = 0 defaults to USB

		// Clock source control
		if ( (frequency == FREQ_44) || (frequency == FREQ_88) || (frequency == FREQ_176) ) {
			gpio_set_gpio_pin(AVR32_PIN_PX58); 	// 44.1 control
			gpio_clr_gpio_pin(AVR32_PIN_PX45); 	// 48 control
		}
		else {
			gpio_clr_gpio_pin(AVR32_PIN_PX58); 	// 44.1 control
			gpio_set_gpio_pin(AVR32_PIN_PX45); 	// 48 control
		}
	}
	else if ( (source == MOBO_SRC_SPDIF) || (source == MOBO_SRC_TOSLINK) ) {
		gpio_set_gpio_pin(AVR32_PIN_PX44); 		// SEL_USBN_RXP = 0 defaults to USB
		gpio_clr_gpio_pin(AVR32_PIN_PX58); 		// Disable XOs 44.1 control
		gpio_clr_gpio_pin(AVR32_PIN_PX45); 		// Disable XOs 48 control
	}
#else
#error undefined hardware
#endif
}

// LED control
void mobo_led_select(U32 frequency, uint8_t source) {
	switch (frequency) {
		case FREQ_44:
			if (source == MOBO_SRC_UAC1)
				mobo_led(FLED_DARK, FLED_GREEN, FLED_DARK);		// UAC1 green 010
			if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_DARK, FLED_RED, FLED_DARK);		// UAC2 red 010
			if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_DARK, FLED_YELLOW, FLED_DARK);	// SPDIF yellow 010
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(FLED_DARK, FLED_PURPLE, FLED_DARK);	// TOSLINK purple 010
		break;
		case FREQ_48:
			if (source == MOBO_SRC_UAC1)
				mobo_led(FLED_DARK, FLED_GREEN, FLED_GREEN);	// UAC1 green 011
			if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_DARK, FLED_RED, FLED_RED);		// UAC2 red 011
			if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_DARK, FLED_YELLOW, FLED_YELLOW);	// SPDIF yellow 011
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(FLED_DARK, FLED_PURPLE, FLED_PURPLE);	// TOSLINK purple 011
		break;
		case FREQ_88:
			if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_RED, FLED_DARK, FLED_DARK);		// UAC2 red 100
			if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_YELLOW, FLED_DARK, FLED_DARK);	// SPDIF yellow 100
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(FLED_PURPLE, FLED_DARK, FLED_DARK);	// TOSLINK purple 100
		break;
		case FREQ_96:
			if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_RED, FLED_DARK, FLED_RED);		// UAC2 red 101
			if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_YELLOW, FLED_DARK, FLED_YELLOW);	// SPDIF yellow 101
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(FLED_PURPLE, FLED_DARK, FLED_PURPLE);	// TOSLINK purple 101
		break;
		case FREQ_176:
			if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_RED, FLED_RED, FLED_DARK);		// UAC2 red 110
			if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_YELLOW, FLED_YELLOW, FLED_DARK);	// SPDIF yellow 110
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(FLED_PURPLE, FLED_PURPLE, FLED_DARK);	// TOSLINK purple 110
		break;
		case FREQ_192:
			if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_RED, FLED_RED, FLED_RED);			// UAC2 red 111
			if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_YELLOW, FLED_YELLOW, FLED_YELLOW);	// SPDIF yellow 111
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(FLED_PURPLE, FLED_PURPLE, FLED_PURPLE);	// TOSLINK purple 111
		break;
		default:
			mobo_led(FLED_DARK, FLED_DARK, FLED_DARK);			// Invalid frequency: darkness
		break;
	}
}


//
//-----------------------------------------------------------------------------
// The below structure contains a number of implementation dependent definitions (user tweak stuff)
//-----------------------------------------------------------------------------
//
mobo_data_t	cdata							// Variables in ram/flash rom (default)
		=
		{
					COLDSTART_REF			// Update into eeprom if value mismatch
				,	FALSE					// FALSE if UAC1 Audio, TRUE if UAC2 Audio.
				,	SI570_I2C_ADDRESS		// Si570 I2C address or Si570_I2C_addr
				,	TMP100_I2C_ADDRESS		// I2C address for the TMP100 chip
				,	AD5301_I2C_ADDRESS		// I2C address for the AD5301 DAC chip
				,	AD7991_I2C_ADDRESS		// I2C address for the AD7991 4 x ADC chip
				,	PCF_MOBO_I2C_ADDR		// I2C address for the onboard PCF8574
				,	PCF_LPF1_I2C_ADDR		// I2C address for the first MegaFilterMobo PCF8574
				,	PCF_LPF2_I2C_ADDR		// I2C address for the second MegaFilterMobo PCF8574
				,	PCF_EXT_I2C_ADDR		// I2C address for an external PCF8574 used for FAN, attenuators etc
				,	HI_TMP_TRIGGER			// If PA temperature goes above this point, then
											// disable transmission
				,	P_MIN_TRIGGER			// Min P out measurement for SWR trigger
				,	SWR_PROTECT_TIMER		// Timer loop value (in 10ms increments)
				,	SWR_TRIGGER				// Max SWR threshold (10 x SWR)
				,	PWR_CALIBRATE			// Power meter calibration value
				,	BIAS_SELECT				// Which bias, 0 = Cal, 1 = LO, 2 = HI
				,	BIAS_LO					// PA Bias in 10 * mA, typically  20mA or Class B
				,	BIAS_HI					// PA Bias in 10 * mA, typically 350mA or Class A
				,	CAL_LO					// PA Bias setting, Class LO
				,	CAL_HI					// PA Bias setting, Class HI
				,	DEVICE_XTAL				// FreqXtal
				,	3500					// SmoothTunePPM
				, {	( 7.050000 * _2(23) )	// Freq at startup, Default
				,	( 1.820000 * _2(23) )	// Freq at startup, Memory 1
				,	( 3.520000 * _2(23) )	// Freq at startup, Memory 2
				,	( 7.020000 * _2(23) )	// Freq at startup, Memory 3
				,	(10.120000 * _2(23) )	// Freq at startup, Memory 4
				,	(14.020000 * _2(23) )	// Freq at startup, Memory 5
				,	(18.090000 * _2(23) )	// Freq at startup, Memory 6
				,	(21.020000 * _2(23) )	// Freq at startup, Memory 7
				,	(24.910000 * _2(23) )	// Freq at startup, Memory 8
				,	(28.020000 * _2(23) ) }	// Freq at startup, Memory 9
				, 	3						// Which memory was last in use
				, {	(  2.0 * 4.0 * _2(5) )	// Default filter cross over
				,	(  4.0 * 4.0 * _2(5) )	// frequencies for Mobo V4.3
				,	(  8.0 * 4.0 * _2(5) )	// BPF. eight value array.
				,	( 11.0 * 4.0 * _2(5) )
				,	( 14.5 * 4.0 * _2(5) )
				,	( 22.0 * 4.0 * _2(5) )
				,	( 25.0 * 4.0 * _2(5) )
				,	( TRUE ) }
				//, ( {  2.0 * 4.0 * _2(5) )	// Default filter crossover
				//,	(  4.0 * 4.0 * _2(5) )	// frequencies for the K5OOR
				//,	(  7.5 * 4.0 * _2(5) )	// HF Superpacker Pro LPF bank
				//,	( 14.5 * 4.0 * _2(5) )	// Six values in an eight value array.
				//,	( 21.5 * 4.0 * _2(5) )
				//,	( 30.0 * 4.0 * _2(5) )	// The highest two values parked above 30 MHz
				//,	( 30.0 * 4.0 * _2(5) )
				//,	( True ) }
				#if PCF_LPF || PCF_FILTER_IO// 8x BCD control for LPF switching, switches P1 pins 4-6
				, { (  2.0 * 4.0 * _2(5) )	// Default filter crossover
				,	(  4.0 * 4.0 * _2(5) )	// frequencies as per Alex email
				,	(  8.0 * 4.0 * _2(5) )	// 2009-08-15
				,	( 11.0 * 4.0 * _2(5) )
				,	( 14.5 * 4.0 * _2(5) )
				,	( 18.2 * 4.0 * _2(5) )
				,	( 21.0 * 4.0 * _2(5) )
				,	( TRUE ) }
				#elif M0RZF_FILTER_IO		// M0RZF 20W amplifier LPF switching, switches P1 pins 4-6
				, { (  5.0 * 4.0 * _2(5) )	// Default filter crossover
				,	(  9.0 * 4.0 * _2(5) )	// frequencies as per M0RZFR
				,	( 15.0 * 4.0 * _2(5) )
				,	( TRUE ) }
				#else
				, { (  2.0 * 4.0 * _2(5) )	// Default filter crossover
				,	(  4.0 * 4.0 * _2(5) )	// frequencies as per Alex email
				,	(  8.0 * 4.0 * _2(5) )	// 2009-08-15
				,	( 11.0 * 4.0 * _2(5) )
				,	( 14.5 * 4.0 * _2(5) )
				,	( 18.2 * 4.0 * _2(5) )
				,	( 21.0 * 4.0 * _2(5) )
				,	( 30.0 * 4.0 * _2(5) )
				,	( 31.0 * 4.0 * _2(5) )
				,	( 32.0 * 4.0 * _2(5) )
				,	( 33.0 * 4.0 * _2(5) )
				,	( 34.0 * 4.0 * _2(5) )
				,	( 35.0 * 4.0 * _2(5) )
				,	( 36.0 * 4.0 * _2(5) )
				,	( 37.0 * 4.0 * _2(5) )
				,	( TRUE ) }
				#endif
				,	PWR_FULL_SCALE			// Full Scale setting for Power Output Bargraph, in W
				,	SWR_FULL_SCALE			// Full Scale setting for SWR Bargraph,
											// (Max SWR = Value + 1, or 4 = SWR of 5.0)
				,	PEP_PERIOD				// Number of samples in PEP measurement
				,	ENC_PULSES				// Number of Resolvable States per Revolution
				,	1						// VFO Resolution 1/2/5/10/50/100kHz per revolution
				,	0						// PSDR-IQ Freq offset value is in +/- kHz
				,	45						// Fan On trigger temp in degrees C
				,	40						// Fan Off trigger temp in degrees C
				,	PCF_EXT_FAN_BIT			// Which bit is used to control the Cooling Fan
				#if SCRAMBLED_FILTERS		// Enable a non contiguous order of filters
				,	{ Mobo_PCF_FLT0			// Band Pass filter selection
				,	  Mobo_PCF_FLT1			// these values are mapped against the result of the
				,	  Mobo_PCF_FLT2			// filter crossover point comparison
				,	  Mobo_PCF_FLT3			// Filter selected by writing value to output port
				,	  Mobo_PCF_FLT4
				,	  Mobo_PCF_FLT5
				,	  Mobo_PCF_FLT6
				,	  Mobo_PCF_FLT7	}
				,	{ I2C_EXTERN_FLT0		// External LPF filter selection
				,	  I2C_EXTERN_FLT1		// these values are mapped against the result of the
				,	  I2C_EXTERN_FLT2		// filter crossover point comparison
				,	  I2C_EXTERN_FLT3		// Value is used to set 1 out of 16 bits in a double
				,	  I2C_EXTERN_FLT4		// 8bit port (2x PCF8574 GPIO)
				,	  I2C_EXTERN_FLT5
				,	  I2C_EXTERN_FLT6
				,	  I2C_EXTERN_FLT7
				,	  I2C_EXTERN_FLT8
				,	  I2C_EXTERN_FLT9
				,	  I2C_EXTERN_FLTa
				,	  I2C_EXTERN_FLTb
				,	  I2C_EXTERN_FLTc
				,	  I2C_EXTERN_FLTd
				,	  I2C_EXTERN_FLTe
				,	  I2C_EXTERN_FLTf }
				#endif
				#if CALC_FREQ_MUL_ADD		// Frequency Subtract and Multiply Routines (for Smart VFO)
				,	0.000 * _2(21)			// Freq subtract value is 0.0MHz (11.21bits)
				,	1.000 * _2(21)			// Freq multiply value os 1.0    (11.21bits)
				#endif
				#if CALC_BAND_MUL_ADD		// Frequency Subtract and Multiply Routines (for smart VFO)
				,	{ 0.000 * _2(21)		// Freq subtract value is 0.0MHz (11.21bits)
				,	  0.000 * _2(21)
				,	  0.000 * _2(21)
				,	  0.000 * _2(21)
				,	  0.000 * _2(21)
				,	  0.000 * _2(21)
				,	  0.000 * _2(21)
				,	  0.000 * _2(21) }
				,	{ 1.000 * _2(21)		// Freq multiply value is 1.0MHz (11.21bits)
				,	  1.000 * _2(21)
				,	  1.000 * _2(21)
				,	  1.000 * _2(21)
				,	  1.000 * _2(21)
				,	  1.000 * _2(21)
				,	  1.000 * _2(21)
				,	  1.000 * _2(21) }
				#endif
		};

