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

// Power module and clock control
#include "pm.h"

// To access global input source variable
#include "device_audio_task.h"

// To access DAC_BUFFER_SIZE and clear audio buffer
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

#ifdef HW_GEN_DIN20

// Control headphone amp power supply (K-mult) turn-on time, for now main VDD turn on/off!
void mobo_km(uint8_t enable) {
	if (enable == MOBO_HP_KM_ENABLE)
		gpio_set_gpio_pin(AVR32_PIN_PX55);				// Clear PX55 to no longer short the KM capacitors
	else
		gpio_clr_gpio_pin(AVR32_PIN_PX55);				// Set PX55 to short the KM capacitors (pulled up on HW)
}

// Control USB multiplexer in HW_GEN_DIN20
void mobo_usb_select(uint8_t usb_ch) {
	if (usb_ch == USB_CH_NONE) {
		gpio_set_gpio_pin(USB_DATA_ENABLE_PIN_INV);		// Disable USB MUX
		gpio_clr_gpio_pin(AVR32_PIN_PA28);				// NO USB B to MCU's VBUS pin
		gpio_clr_gpio_pin(AVR32_PIN_PA31);				// NO USB A to MCU's VBUS pin
	}
	if (usb_ch == USB_CH_A) {
		gpio_clr_gpio_pin(USB_VBUS_B_PIN);				// NO USB B to MCU's VBUS pin
		gpio_clr_gpio_pin(USB_DATA_ENABLE_PIN_INV);		// Enable USB MUX
		gpio_clr_gpio_pin(USB_DATA_A0_B1_PIN);			// Select USB A to MCU's USB data pins
		gpio_set_gpio_pin(USB_VBUS_A_PIN);				// Select USB A to MCU's VBUS pin
	}
	if (usb_ch == USB_CH_B) {
		gpio_clr_gpio_pin(USB_VBUS_A_PIN);				// NO USB A to MCU's VBUS pin
		gpio_clr_gpio_pin(USB_DATA_ENABLE_PIN_INV);		// Enable USB MUX
		gpio_set_gpio_pin(USB_DATA_A0_B1_PIN);			// Select USB B to MCU's USB data pins
		gpio_set_gpio_pin(USB_VBUS_B_PIN);				// Select USB B to MCU's VBUS pin
	}
}

// Quick and dirty detect of whether front USB (A) is plugged in. No debounce here!
uint8_t mobo_usb_detect(void) {
	if  (gpio_get_pin_value(AVR32_PIN_PB11) == 1)
		return USB_CH_A;

	return USB_CH_B;
}

void  mobo_i2s_enable(uint8_t i2s_mode) {
	if (i2s_mode == MOBO_I2S_ENABLE) {
		gpio_set_gpio_pin(AVR32_PIN_PX11); 					// Enable I2S data
#ifdef USB_STATE_MACHINE_DEBUG
//		print_dbg_char('m');								// Indicate unmute
#endif
	}
	else if (i2s_mode == MOBO_I2S_DISABLE) {
		gpio_clr_gpio_pin(AVR32_PIN_PX11); 					// Disable I2S data pin
#ifdef USB_STATE_MACHINE_DEBUG
//		print_dbg_char('M');								// Indicate mute
#endif
	}
}

#endif


#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)

// Audio Widget HW_GEN_DIN10 / DIN20 LED control
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

// Front panel RGB LED control
void mobo_led_select(U32 frequency, uint8_t source) {
	switch (frequency) {

		case FREQ_44:
#ifdef HW_GEN_DIN10
			if (source == MOBO_SRC_UAC1)
				mobo_led(FLED_DARK, FLED_GREEN, FLED_DARK);		// UAC1 green 010
			else if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_DARK, FLED_RED, FLED_DARK);		// UAC2 red 010
#endif
#ifdef HW_GEN_DIN20
			if ( (source == MOBO_SRC_UAC1) && (usb_ch == USB_CH_B) )
				mobo_led(FLED_DARK, FLED_GREEN, FLED_DARK);		// UAC1 rear green 010
			else if ( (source == MOBO_SRC_UAC1) && (usb_ch == USB_CH_A) )
				mobo_led(FLED_DARK, FLED_BLUE, FLED_DARK);		// UAC1 front blue 010
			else if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_B) )
				mobo_led(FLED_DARK, FLED_RED, FLED_DARK);		// UAC2 rear red 010
			else if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_A) )
				mobo_led(FLED_DARK, FLED_WHITE, FLED_DARK);		// UAC2 front white 010
#endif
			else if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_DARK, FLED_PURPLE, FLED_DARK);	// SPDIF purple 010
			else if (source == MOBO_SRC_TOS2)
				mobo_led(FLED_DARK, FLED_CYAN, FLED_DARK);		// TOS2 cyan 010
			else if (source == MOBO_SRC_TOS1)
				mobo_led(FLED_DARK, FLED_YELLOW, FLED_DARK);	// TOS1 yellow 010
		break;

		case FREQ_48:
#ifdef HW_GEN_DIN10
			if (source == MOBO_SRC_UAC1)
				mobo_led(FLED_DARK, FLED_GREEN, FLED_GREEN);	// UAC1 green 011
			else if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_DARK, FLED_RED, FLED_RED);		// UAC2 red 011
#endif
#ifdef HW_GEN_DIN20
			if ( (source == MOBO_SRC_UAC1) && (usb_ch == USB_CH_B) )
				mobo_led(FLED_DARK, FLED_GREEN, FLED_GREEN);	// UAC1 rear green 011
			else if ( (source == MOBO_SRC_UAC1) && (usb_ch == USB_CH_A) )
				mobo_led(FLED_DARK, FLED_BLUE, FLED_BLUE);		// UAC1 front blue 011
			else if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_B) )
				mobo_led(FLED_DARK, FLED_RED, FLED_RED);		// UAC2 rear red 011
			else if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_A) )
				mobo_led(FLED_DARK, FLED_WHITE, FLED_WHITE);	// UAC2 front white 011
#endif
			else if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_DARK, FLED_PURPLE, FLED_PURPLE);	// SPDIF purple 011
			else if (source == MOBO_SRC_TOS2)
				mobo_led(FLED_DARK, FLED_CYAN, FLED_CYAN);		// TOS2 cyan 011
			else if (source == MOBO_SRC_TOS1)
				mobo_led(FLED_DARK, FLED_YELLOW, FLED_YELLOW);	// TOS1 yellow 011
		break;

		case FREQ_88:
#ifdef HW_GEN_DIN10
			if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_RED, FLED_DARK, FLED_DARK);		// UAC2 red 100
#endif
#ifdef HW_GEN_DIN20
			if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_B) )
				mobo_led(FLED_RED, FLED_DARK, FLED_DARK);		// UAC2 rear red 100
			else if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_A) )
				mobo_led(FLED_WHITE, FLED_DARK, FLED_DARK);		// UAC2 front white 100
#endif
			else if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_PURPLE, FLED_DARK, FLED_DARK);	// SPDIF purple 100
			else if (source == MOBO_SRC_TOS2)
				mobo_led(FLED_CYAN, FLED_DARK, FLED_DARK);		// TOS2 cyan 100
			else if (source == MOBO_SRC_TOS1)
				mobo_led(FLED_YELLOW, FLED_DARK, FLED_DARK);	// TOS1 yellow 100
		break;

		case FREQ_96:
#ifdef HW_GEN_DIN10
			if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_RED, FLED_DARK, FLED_RED);		// UAC2 red 101
#endif
#ifdef HW_GEN_DIN20
			if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_B) )
				mobo_led(FLED_RED, FLED_DARK, FLED_RED);		// UAC2 rear red 101
			else if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_A) )
				mobo_led(FLED_WHITE, FLED_DARK, FLED_WHITE);	// UAC2 front white 101
#endif
			else if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_PURPLE, FLED_DARK, FLED_PURPLE);	// SPDIF purple 101
			else if (source == MOBO_SRC_TOS2)
				mobo_led(FLED_CYAN, FLED_DARK, FLED_CYAN);		// TOS2 cyan 101
			else if (source == MOBO_SRC_TOS1)
				mobo_led(FLED_YELLOW, FLED_DARK, FLED_YELLOW);	// TOS1 yellow 101
		break;

		case FREQ_176:
#ifdef HW_GEN_DIN10
			if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_RED, FLED_RED, FLED_DARK);		// UAC2 red 110
#endif
#ifdef HW_GEN_DIN20
			if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_B) )
				mobo_led(FLED_RED, FLED_RED, FLED_DARK);		// UAC2 rear red 110
			else if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_A) )
				mobo_led(FLED_WHITE, FLED_WHITE, FLED_DARK);	// UAC2 front white 110
#endif
			else if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_PURPLE, FLED_PURPLE, FLED_DARK);	// SPDIF purple 110
			else if (source == MOBO_SRC_TOS2)
				mobo_led(FLED_CYAN, FLED_CYAN, FLED_DARK);		// TOS2 cyan 110
			else if (source == MOBO_SRC_TOS1)
				mobo_led(FLED_YELLOW, FLED_YELLOW, FLED_DARK);	// TOS1 yellow 110
		break;

		case FREQ_192:
#ifdef HW_GEN_DIN10
			if (source == MOBO_SRC_UAC2)
				mobo_led(FLED_RED, FLED_RED, FLED_RED);				// UAC2 red 111
#endif
#ifdef HW_GEN_DIN20
			if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_B) )
				mobo_led(FLED_RED, FLED_RED, FLED_RED);				// UAC2 rear red 111
			else if ( (source == MOBO_SRC_UAC2) && (usb_ch == USB_CH_A) )
				mobo_led(FLED_WHITE, FLED_WHITE, FLED_WHITE);		// UAC2 front white 111
#endif
			else if (source == MOBO_SRC_SPDIF)
				mobo_led(FLED_PURPLE, FLED_PURPLE, FLED_PURPLE);	// SPDIF purple 111
			else if (source == MOBO_SRC_TOS2)
				mobo_led(FLED_CYAN, FLED_CYAN, FLED_CYAN);			// TOS2 cyan 111
			else if (source == MOBO_SRC_TOS1)
				mobo_led(FLED_YELLOW, FLED_YELLOW, FLED_YELLOW);	// TOS1 yellow 111
		break;

		default:
			mobo_led(FLED_DARK, FLED_DARK, FLED_DARK);			// Invalid frequency: darkness
		break;
	}
}


// Handle spdif and toslink input
void mobo_handle_spdif(uint8_t width) {
	static int ADC_buf_DMA_write_prev = -1;
	int ADC_buf_DMA_write_temp = 0;
	static U32 spk_index = 0;
	static S16 gap = DAC_BUFFER_SIZE;
	S16 old_gap = DAC_BUFFER_SIZE;

	U16 samples_to_transfer_OUT = 1; 	// Default value 1. Skip:0. Insert:2
	S16 i;								// Generic counter
	S16 p;								// Generic counter

	U8 DAC_buf_DMA_read_local;			// Local copy read in atomic operations
	U16 num_remaining;

	S32 sample_temp = 0;
	static S32 sample_L = 0;
	static S32 sample_R = 0;
	static S16 megaskip = 0;
	S16 target = -1;					// Default value, no sample to touch


// The Henry Audio and QNKTC series of hardware only use NORMAL I2S with left before right
#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20) || (defined HW_GEN_AB1X)
#define IN_LEFT 0
#define IN_RIGHT 1
#define OUT_LEFT 0
#define OUT_RIGHT 1
#else
	const U8 IN_LEFT = FEATURE_IN_NORMAL ? 0 : 1;
	const U8 IN_RIGHT = FEATURE_IN_NORMAL ? 1 : 0;
	const U8 OUT_LEFT = FEATURE_OUT_NORMAL ? 0 : 1;
	const U8 OUT_RIGHT = FEATURE_OUT_NORMAL ? 1 : 0;
#endif


	ADC_buf_DMA_write_temp = ADC_buf_DMA_write; // Interrupt may strike at any time!

	// Continue writing to consumer's buffer where this routine left of last
	if ( (ADC_buf_DMA_write_prev == -1)	|| (ADC_buf_USB_IN == -1) )	 {	// Do the init on synchronous sampling ref. ADC DMA timing
		// Clear incoming SPDIF before enabling pdca to keep filling it
		for (i = 0; i < ADC_BUFFER_SIZE; i++) {
			audio_buffer_0[i] = 0;
			audio_buffer_1[i] = 0;
		}

		ADC_buf_DMA_write_prev = ADC_buf_DMA_write_temp;
		ADC_buf_USB_IN = -2;
	}

	if (wm8805_status.reliable == 0) { // Temporarily unreliable counts as silent and halts processing
		wm8805_status.silent = 1;
		ADC_buf_DMA_write_prev = ADC_buf_DMA_write_temp;			// Respond as soon as .reliable is set
	}

	// Has producer's buffer been toggled by interrupt driven DMA code?
	// If so, check it for silence. If selected as source, copy all of producer's data. (And perform skip/insert.)
	// Only bother if .reliable != 0
	else if (wm8805_status.buffered == 0) {
		wm8805_status.silent = 0;
	}
	else if (ADC_buf_DMA_write_temp != ADC_buf_DMA_write_prev) { // Check if producer has sent more data
		ADC_buf_DMA_write_prev = ADC_buf_DMA_write_temp;

		// Silence / DC detector 2.0
//			if (wm8805_status.reliable == 1) {			// This code is unable to detect silence in a shut-down WM8805
			for (i=0 ; i < ADC_BUFFER_SIZE ; i++) {
				if (ADC_buf_DMA_write_temp == 0)	// End as soon as a difference is spotted
					sample_temp = audio_buffer_0[i] & 0x00FFFF00;
				else if (ADC_buf_DMA_write_temp == 1)
					sample_temp = audio_buffer_1[i] & 0x00FFFF00;

				if ( (sample_temp != 0x00000000) && (sample_temp != 0x00FFFF00) ) // "zero" according to tested sources
					i = ADC_BUFFER_SIZE + 10;
			}

			if (i >= ADC_BUFFER_SIZE + 10) {		// Non-silence was detected
				wm8805_status.silent = 0;
			}
			else {									// Silence was detected, update flag to SPDIF RX code
				wm8805_status.silent = 1;
			}
//			}

		if ( ( (input_select == MOBO_SRC_TOS1) || (input_select == MOBO_SRC_TOS2) || (input_select == MOBO_SRC_SPDIF) ) ) {

			// Startup condition: must initiate consumer's write pointer to where-ever its read pointer may be
			if (ADC_buf_USB_IN == -2) {
				ADC_buf_USB_IN = ADC_buf_DMA_write_temp;	// Disable further init
				dac_must_clear = DAC_READY;					// Prepare to send actual data to DAC interface

				// USB code has !0 detection, semaphore checks etc. etc. around here. See line 744 in uac2_dat.c
//				skip_enable = 0;
				gap = DAC_BUFFER_SIZE; // Ideal gap value

				// New co-sample verification routine
				DAC_buf_DMA_read_local = DAC_buf_DMA_read;
				num_remaining = spk_pdca_channel->tcr;
				// Did an interrupt strike just there? Check if DAC_buf_DMA_read is valid. If not, interrupt won't strike again
				// for a long time. In which we simply read the counter again
				if (DAC_buf_DMA_read_local != DAC_buf_DMA_read) {
					DAC_buf_DMA_read_local = DAC_buf_DMA_read;
					num_remaining = spk_pdca_channel->tcr;
				}
				DAC_buf_USB_OUT = DAC_buf_DMA_read_local;

				spk_index = DAC_BUFFER_SIZE - num_remaining;
//				spk_index = DAC_BUFFER_SIZE - num_remaining + DAC_BUFFER_SIZE * 0.9/4; // Nasty offset to put on skips sooner
				spk_index = spk_index & ~((U32)1); 	// Clear LSB in order to start with L sample
			}

			// Calculate gap before copying data into consumer register:
			old_gap = gap;

			// New co-sample verification routine
			// FIX: May Re-introduce code which samples DAC_buf_DMA_read and num_remaining as part of ADC DMA interrupt routine?
			// If so look for "BUF_IS_ONE" in code around 20170227
			DAC_buf_DMA_read_local = DAC_buf_DMA_read;
			num_remaining = spk_pdca_channel->tcr;
			// Did an interrupt strike just there? Check if DAC_buf_DMA_read is valid. If not, interrupt won't strike again
			// for a long time. In which we simply read the counter again
			if (DAC_buf_DMA_read_local != DAC_buf_DMA_read) {
				DAC_buf_DMA_read_local = DAC_buf_DMA_read;
				num_remaining = spk_pdca_channel->tcr;
			}

			if (DAC_buf_USB_OUT != DAC_buf_DMA_read_local) { 	// DAC DMA and seq. code using same buffer
				if (spk_index < (DAC_BUFFER_SIZE - num_remaining))
					gap = DAC_BUFFER_SIZE - num_remaining - spk_index;
				else
					gap = DAC_BUFFER_SIZE - spk_index + DAC_BUFFER_SIZE - num_remaining + DAC_BUFFER_SIZE;
			}
			else // DAC DMA and seq. code working on different buffers
				gap = (DAC_BUFFER_SIZE - spk_index) + (DAC_BUFFER_SIZE - num_remaining);


			// Apply gap to skip or insert, for now we're not reusing skip_enable from USB coee
			samples_to_transfer_OUT = 1;			// Default value
			target = -1;
			if ((gap <= old_gap) && (gap < SPK_GAP_L3)) {
//			if ((gap <= old_gap) && (gap < SPK_GAP_L1)) {
//			if (gap < SPK_GAP_L1) {
				samples_to_transfer_OUT = 0;		// Do some skippin'
//				print_dbg_char('s');
			}
			else if ((gap >= old_gap) && (gap > SPK_GAP_U3)) {
//			else if ((gap >= old_gap) && (gap > SPK_GAP_U1)) {
//			else if (gap > SPK_GAP_U1) {
				samples_to_transfer_OUT = 2;		// Do some insertin'
//				print_dbg_char('i');
			}



			// If we must skip, what is the best place to do that?
			// Code is prototyped in skip_insert_draft_c.m
			if (samples_to_transfer_OUT != 1) {
				target = 0;					// If calculation fails, remove 1st sample in package
				S32 prevsample_L = sample_L; 	// sample_L is static, so this is the last data from previous package transfer
				S32 prevsample_R = sample_R;
				if (width == 24) {				// We're starting out with 32-bit signed math here.
					prevsample_L <<= 8;
					prevsample_R <<= 8;
				}
				prevsample_L >>= 4;
				prevsample_R >>= 4;
				S32 absdiff_L = 0;
				S32 absdiff_R = 0;
				S32 prevabsdiff_L = 0;
				S32 prevabsdiff_R = 0;
				S32 score = 0;
				S32 prevscore = 0x7FFFFFFF;	// An unreasonably large positive number

				for (i=0 ; i < ADC_BUFFER_SIZE ; i+=2) {
					if (ADC_buf_DMA_write_temp == 0) {		// 0 Seems better than 1, but non-conclusive
						sample_L = audio_buffer_0[i+IN_LEFT];
						sample_R = audio_buffer_0[i+IN_RIGHT];
					}
					else if (ADC_buf_DMA_write_temp == 1) {
						sample_L = audio_buffer_1[i+IN_LEFT];
						sample_R = audio_buffer_1[i+IN_RIGHT];
					}
					if (width == 24) {			// We're starting out with 32-bit signed math here.
						sample_L <<= 8;
						sample_R <<= 8;
					}

					// Calculating the "energy" coming from sample n-1 to sample n
					sample_L >>= 4;	// Avoid saturation
					sample_R >>= 4;
					absdiff_L = abs(sample_L - prevsample_L);
					absdiff_R = abs(sample_R - prevsample_R);

					// Summing the "energy" going from sample n-2 to sample n-1 and the energy going from sample n-1 to sample n
					// Determine which stereo sample should be touched
					score = absdiff_L + absdiff_R + prevabsdiff_L + prevabsdiff_R;
					if (score < prevscore) {
						if (i != 0) {		// Can't touch last sample of package
							target = i-2;	// Stereo sample offset
							prevscore = score;
						}
					}

					// Establish history within packet. Redundant in very last sample in package, but if test is too expensive
					prevsample_L = sample_L;
					prevsample_R = sample_R;
					prevabsdiff_L = absdiff_L;
					prevabsdiff_R = absdiff_R;
				}
#ifdef USB_STATE_MACHINE_DEBUG
//				print_dbg_char_hex(target);
//				print_dbg_char(' ');
#endif
			}

/* Bring this chunk back in again!
			// Prepare to copy all of producer's most recent data to consumer's buffer
			if (ADC_buf_DMA_write_temp == 1)
				gpio_set_gpio_pin(AVR32_PIN_PX18);			// Pin 84
			else if (ADC_buf_DMA_write_temp == 0)
				gpio_clr_gpio_pin(AVR32_PIN_PX18);			// Pin 84
*/
			// Apply megaskip when DC or zero is detected
			if (wm8805_status.silent == 1) {									// Silence was detected
				if (gap < (SPK_GAP_L3 + SPK_GAP_D1) ) {				// Are we close or past the limit for having to skip?
					megaskip = (SPK_GAP_U3 - SPK_GAP_D1) - (gap);	// This is as far as we can safely skip, one ADC package at a time
				}
				else if (gap > (SPK_GAP_U3 - SPK_GAP_D1) ) {			// Are we close to or past the limit for having to insert?
					megaskip = (gap) - (SPK_GAP_L3 + SPK_GAP_D1);	// This is as far as we can safely insert, one ADC package at a time
				}
			}
			else {
				megaskip = 0;	// Not zero -> no big skips!
			}


			// We're skipping or about to skip. In case of silence, do a good and proper skip by copying nothing
			if (megaskip >= ADC_BUFFER_SIZE) {
				print_dbg_char('S');
				samples_to_transfer_OUT = 1; 	// Revert to default:1. I.e. only one skip or insert in next ADC package
				megaskip -= ADC_BUFFER_SIZE;	// We have jumped over one whole ADC package
				// FIX: Is there a need to null the buffers and avoid re-use of old DAC buffer content?
			}
			// We're inserting or about to insert. In case of silence, do a good and proper insert by doubling an ADC package
			else if (megaskip <= -ADC_BUFFER_SIZE) {
				print_dbg_char('I');
				samples_to_transfer_OUT = 1; // Revert to default:1. I.e. only one skip or insert per USB package
				megaskip += ADC_BUFFER_SIZE;	// Prepare to -insert- one ADC package, i.e. copying two ADC packages

				for (i=0 ; i < ADC_BUFFER_SIZE *2 ; i+=2) { // Mind the *2
					if (dac_must_clear == DAC_READY) {
						if (DAC_buf_USB_OUT == 0) {
							spk_buffer_0[spk_index+OUT_LEFT] = 0;
							spk_buffer_0[spk_index+OUT_RIGHT] = 0;
						}
						else if (DAC_buf_USB_OUT == 1) {
							spk_buffer_1[spk_index+OUT_LEFT] = 0;
							spk_buffer_1[spk_index+OUT_RIGHT] = 0;
						}
					}

					spk_index += 2;
					if (spk_index >= DAC_BUFFER_SIZE) {
						spk_index -= DAC_BUFFER_SIZE;
						DAC_buf_USB_OUT = 1 - DAC_buf_USB_OUT;
					}
				} // for i..

			} // mega-insert <=
			// Normal operation, copy one ADC package with normal skip/insert
			else {
				megaskip = 0;					// Normal operation

//				print_dbg_char_hex(target);
//				print_dbg_char('\n');

				for (i=0 ; i < ADC_BUFFER_SIZE ; i+=2) {
					// Fill endpoint with sample raw
					if (ADC_buf_DMA_write_temp == 0) {		// 0 Seems better than 1, but non-conclusive
						sample_L = audio_buffer_0[i+IN_LEFT];
						sample_R = audio_buffer_0[i+IN_RIGHT];
					}
					else if (ADC_buf_DMA_write_temp == 1) {
						sample_L = audio_buffer_1[i+IN_LEFT];
						sample_R = audio_buffer_1[i+IN_RIGHT];
					}


// Super-rough skip/insert
//					while (samples_to_transfer_OUT-- > 0) { // Default:1 Skip:0 Insert:2 Apply to 1st stereo sample in packet

					p = 1;
					if (i == target) {			// Are we touching the stereo sample?
						p = samples_to_transfer_OUT;				// If so let's check what we're doing to it
					}

					while (p-- > 0) { // Default:1 Skip:0 Insert:2 Apply to 1st stereo sample in packet
						if (dac_must_clear == DAC_READY) {
							if (DAC_buf_USB_OUT == 0) {
								spk_buffer_0[spk_index+OUT_LEFT] = sample_L;
								spk_buffer_0[spk_index+OUT_RIGHT] = sample_R;
							}
							else if (DAC_buf_USB_OUT == 1) {
								spk_buffer_1[spk_index+OUT_LEFT] = sample_L;
								spk_buffer_1[spk_index+OUT_RIGHT] = sample_R;
							}
						}

						spk_index += 2;
						if (spk_index >= DAC_BUFFER_SIZE) {
							spk_index -= DAC_BUFFER_SIZE;
							DAC_buf_USB_OUT = 1 - DAC_buf_USB_OUT;

/* Bring this chunk back in again!
#ifdef USB_STATE_MACHINE_DEBUG
							if (DAC_buf_USB_OUT == 1)
								gpio_set_gpio_pin(AVR32_PIN_PX30);
							else
								gpio_clr_gpio_pin(AVR32_PIN_PX30);
#endif
*/
						}
					}
					samples_to_transfer_OUT = 1; // Revert to default:1. I.e. only one skip or insert per USB package
				} // for ADC_BUFFER_SIZE
			} // Normal operation

		} // ADC_buf_DMA_write toggle
	} // input select
} // mobo_handle_spdif(void)

#endif

/*! \brief Audio Widget select oscillator
 *
 * \retval none
 */
void mobo_xo_select(U32 frequency, uint8_t source) {
// XO control and SPI muxing on ab1x hardware generation
	static U32 prev_frequency = FREQ_INVALID;

	if (frequency != prev_frequency) {		//   Only run when things change

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

	// XO control and I2S muxing on Digital Input 1.0 / 2.0 generation
	// NB: updated to support SPDIF buffering in MCU. That is highly experimental code!
	#elif ((defined HW_GEN_DIN10) || (defined HW_GEN_DIN20))

		if (wm8805_status.buffered == 0) {
		// Old version with I2S mux
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
			else if ( (source == MOBO_SRC_SPDIF) || (source == MOBO_SRC_TOS2)  || (source == MOBO_SRC_TOS1) ) {
				gpio_set_gpio_pin(AVR32_PIN_PX44); 		// SEL_USBN_RXP = 0 defaults to USB
				gpio_clr_gpio_pin(AVR32_PIN_PX58); 		// Disable XOs 44.1 control
				gpio_clr_gpio_pin(AVR32_PIN_PX45); 		// Disable XOs 48 control
			}
		}
		else {
		// New version without I2S mux, with buffering via MCU's ADC interface
			gpio_clr_gpio_pin(AVR32_PIN_PX44); 			// SEL_USBN_RXP = 0 USB version in all cases

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

	#else
		#error undefined hardware
	#endif

		prev_frequency = frequency;
	} // if (frequency != prev_frequency)
}


// Master clock to DAC's I2S port frequency setup
void mobo_clock_division(U32 frequency) {

	static U32 prev_frequency = FREQ_INVALID;
/*
#ifdef USB_STATE_MACHINE_DEBUG
	if (frequency == FREQ_192)
		print_dbg_char('6');
	else if (frequency == FREQ_176)
		print_dbg_char('5');
	else if (frequency == FREQ_96)
		print_dbg_char('4');
	else if (frequency == FREQ_88)
		print_dbg_char('3');
	else if (frequency == FREQ_48)
		print_dbg_char('2');
	else if (frequency == FREQ_44)
		print_dbg_char('1');
#endif
*/

/*
#ifdef USB_STATE_MACHINE_DEBUG
	print_dbg_char('#');
	if (frequency == FREQ_192)
		print_dbg_char_hex(6);
	else if (frequency == FREQ_176)
		print_dbg_char_hex(5);
	else if (frequency == FREQ_96)
		print_dbg_char_hex(4);
	else if (frequency == FREQ_88)
		print_dbg_char_hex(3);
	else if (frequency == FREQ_48)
		print_dbg_char_hex(2);
	else if (frequency == FREQ_44)
		print_dbg_char_hex(1);
	print_dbg_char('#');
#endif
*/
	if (frequency != prev_frequency) {		// Only run when things change

		gpio_enable_pin_pull_up(AVR32_PIN_PA03);	// Floating: stock AW with external /2. GND: modded AW with no ext. /2

		pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

		// External /2 variety, unmodded hardware with floating, pulled-up PA03 interpreted as 1
		if (gpio_get_pin_value(AVR32_PIN_PA03) == 1) {
			switch (frequency) {
				case FREQ_192 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								0,                  // diven - disabled
								0);                 // not divided
				break;
				case FREQ_176 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,        			// osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,        			// pll_osc: select Osc0/PLL0 or Osc1/PLL1
								0,        			// diven - disabled
								0);                 // not divided
				break;
				case FREQ_96 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								0);                 // divided by 2
				break;
				case FREQ_88 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								0);                 // divided by 2
				break;
				case FREQ_48 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								1);                 // divided by 4
				break;
				case FREQ_44 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								1);                 // divided by 4
				break;
			}
		}

		// No external /2 variety, modded hardware with resistor tying PA03 to 0
		else {
			switch (frequency) {
				case FREQ_192 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								0);                 // divided by 2
				break;
				case FREQ_176 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,        			// osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,        			// pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,        			// diven - enabled
								0);                 // divided by 2
				break;
				case FREQ_96 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								1);                 // divided by 4
				break;
				case FREQ_88 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								1);                 // divided by 4
				break;
				case FREQ_48 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								3);                 // divided by 8
				break;
				case FREQ_44 :
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								3);                 // divided by 8
				break;
			}
		}

		pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

		if (prev_frequency != FREQ_INVALID) 				// Don't run on 1st iteration of this function after bootup
			AK5394A_pdca_tx_enable(frequency);		// Trying to force out LRCK inversion

		prev_frequency = frequency;
	}




// We'd liek to call this one here, but it's not suitable before ssc_i2s_init has run. Or so we believe...
//	AK5394A_pdca_tx_enable();	// Trying to force out LRCK inversion

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
