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

/*
#include "rotary_encoder.h"
#include "AD7991.h"
#include "AD5301.h"
#include "Si570.h"
#include "PCF8574.h"
#include "TMP100.h"
*/

#if defined(HW_GEN_DIN10)

// Sample rate detection test
int16_t mobo_srd(void) {
	int timeout;

	// see srd_test.c and srd_test.lst

	// Determining speed at TP16 / DAC_0P / PA04 for now.

	// Not immediately operational... Is IO pin really indexed correctly? Does it need to be init'ed?
	/*
	 * Maybe:
	 * - Tested frequency is too low?
	 * - IO isn't enabled, datasheet page 383-384, GPER
	 * - 0x60 / 0d96 is the offset for PVR, pin value register, mapping to PA/PB/PX etc. is weird..
	 *
	 */

	gpio_enable_gpio_pin(AVR32_PIN_PA04);	// Enable GPIO pin, not special IO (also for input). Needed?

	asm volatile(
		"ssrf	16				\n\t"	// Disable global interrupt
		"mov	%0, 	8000	\n\t"	// Load timeout
		"mov	r9,		-61440	\n\t"	// Immediate load, set up pointer to PA04, (0xFFFF1000) recompile C for other IO pin, do once

		"L0:					\n\t"	// Loop while PA04 is 1
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	L0_done			\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L0				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L0_done:				\n\t"

		"L1:					\n\t"	// Loop while PA04 is 0
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	L1_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L1				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L1_done:				\n\t"

		"mov	%0, 	8000	\n\t"	// Restart countdon for actual timing of below half-cycles

		"L3:					\n\t"	// Loop while PBA04 is 1
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	L3_done			\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L3				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L3_done:				\n\t"

		"L4:					\n\t"	// Loop while PA04 is 0
		"ld.w	r8,		r9[96]	\n\t"	// Load PA04 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	4		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	L4_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	L4				\n\t"	// Not done counting down
		"rjmp	COUNTD			\n\t"	// Countdown reached
		"L4_done:				\n\t"
		"rjmp	RETURN__		\n\t"

		"COUNTD:				\n\t"	// Countdown reached, %0 is 0

		"RETURN__:				\n\t"
		"csrf	16				\n\t"	// Enable global interrupt
		:	"=r" (timeout)				// One output register
		:								// No input registers
		:	"r8", "r9"					// Clobber registers, pushed/popped unless assigned by GCC as temps
	);

	return timeout;
}





// Audio Widget HW_GEN_DIN10 LED control
void mobo_led (uint8_t fled2, uint8_t fled1, uint8_t fled0) {
	// red:1, green:2, blue:4
	// fled2 is towards center of box, fled0 towards right-hand edge of front view

	if (fled0 & 1)
		gpio_clr_gpio_pin(AVR32_PIN_PA17); 	// FLED0_R
	else
		gpio_set_gpio_pin(AVR32_PIN_PA17); 	// FLED0_R
	if (fled0 & 2)
		gpio_clr_gpio_pin(AVR32_PIN_PA24); 	// FLED0_G
	else
		gpio_set_gpio_pin(AVR32_PIN_PA24); 	// FLED0_G
	if (fled0 & 4)
		gpio_clr_gpio_pin(AVR32_PIN_PA18); 	// FLED0_B
	else
		gpio_set_gpio_pin(AVR32_PIN_PA18); 	// FLED0_B

	if (fled1 & 1)
		gpio_clr_gpio_pin(AVR32_PIN_PA23); 	// FLED1_R
	else
		gpio_set_gpio_pin(AVR32_PIN_PA23); 	// FLED1_R
	if (fled1 & 2)
		gpio_clr_gpio_pin(AVR32_PIN_PC01); 	// FLED1_G
	else
		gpio_set_gpio_pin(AVR32_PIN_PC01); 	// FLED1_G
	if (fled1 & 4)
		gpio_clr_gpio_pin(AVR32_PIN_PA21); 	// FLED1_B
	else
		gpio_set_gpio_pin(AVR32_PIN_PA21); 	// FLED1_B

	if (fled2 & 1)
		gpio_clr_gpio_pin(AVR32_PIN_PX29); 	// FLED2_R
	else
		gpio_set_gpio_pin(AVR32_PIN_PX29); 	// FLED2_R
	if (fled2 & 2)
		gpio_clr_gpio_pin(AVR32_PIN_PX32); 	// FLED2_G
	else
		gpio_set_gpio_pin(AVR32_PIN_PX32); 	// FLED2_G
	if (fled2 & 4)
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


// XO control on ab1x hardware generation
#if defined(HW_GEN_AB1X)
	switch (frequency) {
		case 44100:
			if (FEATURE_BOARD_USBI2S)
				gpio_clr_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 22.5792MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_clr_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL0);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL1);
		break;
		case 48000:
			if (FEATURE_BOARD_USBI2S)
				gpio_set_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 24.576MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_set_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL0);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL1);
		break;
		case 88200:
			if (FEATURE_BOARD_USBI2S)
				gpio_clr_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 22.5792MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_clr_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL1);
			gpio_set_gpio_pin(SAMPLEFREQ_VAL0);
		break;
		case 96000:
			if (FEATURE_BOARD_USBI2S)
				gpio_set_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 24.576MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_set_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL1);
			gpio_set_gpio_pin(SAMPLEFREQ_VAL0);
		break;
		case 176400:
			if (FEATURE_BOARD_USBI2S)
				gpio_clr_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 22.5792MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_clr_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL0);
			gpio_set_gpio_pin(SAMPLEFREQ_VAL1);
		break;
		case 192000:
			if (FEATURE_BOARD_USBI2S)
				gpio_set_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 24.576MHz/2 for AB-1
			else if (FEATURE_BOARD_USBDAC)
				gpio_set_gpio_pin(AVR32_PIN_PX51);
			gpio_clr_gpio_pin(SAMPLEFREQ_VAL0);
			gpio_set_gpio_pin(SAMPLEFREQ_VAL1);
		break;
	}

#elif defined(HW_GEN_DIN10)
	// FIX: correlate with mode currently selected by user or auto, that's a global variable!
	if ( (source == MOBO_SRC_UAC1) || (source == MOBO_SRC_UAC2) || (source == MOBO_SRC_NONE) ) {
		gpio_clr_gpio_pin(AVR32_PIN_PX44); 			// SEL_USBN_RXP = 0 defaults to USB

		// Clock source control
		if ( (frequency == 44100) || (frequency == 88200) || (frequency == 176400) ) {
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

	// LED control
	switch (frequency) {
		case 44100:
			if (source == MOBO_SRC_UAC1)
				mobo_led(0, 2, 0);				// UAC1 green 010
			if (source == MOBO_SRC_UAC2)
				mobo_led(0, 1, 0);				// UAC2 red 010
			if (source == MOBO_SRC_SPDIF)
				mobo_led(0, 3, 0);				// SPDIF yellow 010
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(0, 5, 0);				// TOSLINK purple 010
		break;
		case 48000:
			if (source == MOBO_SRC_UAC1)
				mobo_led(0, 2, 2);				// UAC1 green 011
			if (source == MOBO_SRC_UAC2)
				mobo_led(0, 1, 1);				// UAC2 red 011
			if (source == MOBO_SRC_SPDIF)
				mobo_led(0, 3, 3);				// SPDIF yellow 011
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(0, 5, 5);				// TOSLINK purple 011
		break;
		case 88200:
			if (source == MOBO_SRC_UAC2)
				mobo_led(1, 0, 0);				// UAC2 red 100
			if (source == MOBO_SRC_SPDIF)
				mobo_led(3, 0, 0);				// SPDIF yellow 100
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(5, 0, 0);				// TOSLINK purple 100
		break;
		case 96000:
			if (source == MOBO_SRC_UAC2)
				mobo_led(1, 0, 1);				// UAC2 red 101
			if (source == MOBO_SRC_SPDIF)
				mobo_led(3, 0, 3);				// SPDIF yellow 101
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(5, 0, 5);				// TOSLINK purple 101
		break;
		case 176400:
			if (source == MOBO_SRC_UAC2)
				mobo_led(1, 1, 0);				// UAC2 red 110
			if (source == MOBO_SRC_SPDIF)
				mobo_led(3, 3, 0);				// SPDIF yellow 110
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(5, 5, 0);				// TOSLINK purple 110
		break;
		case 192000:
			if (source == MOBO_SRC_UAC2)
				mobo_led(1, 1, 1);				// UAC2 red 111
			if (source == MOBO_SRC_SPDIF)
				mobo_led(3, 3, 3);				// SPDIF yellow 111
			if (source == MOBO_SRC_TOSLINK)
				mobo_led(5, 5, 5);				// TOSLINK purple 111
		break;
	}
#else
#error undefined hardware
#endif


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

