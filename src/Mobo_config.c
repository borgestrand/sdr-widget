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

// XO control and I2S muxing on Digital Input 1.0 / 2.0 generation
// NB: updated to support SPDIF buffering in MCU. That is highly experimental code!
#elif ((defined HW_GEN_DIN10) || (defined HW_GEN_DIN20))

/* Old version with I2S mux
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
*/

// New version without I2S mux, with buffering via MCU's ADC interface
	// FIX: correlate with mode currently selected by user or auto, that's a global variable!
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

#else
#error undefined hardware
#endif
}


// Master clock to DAC's I2S port frequency setup
void mobo_clock_division(U32 frequency) {

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

