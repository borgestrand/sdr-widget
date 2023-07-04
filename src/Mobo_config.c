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

// Real-time counter management
#include "rtc.h"

// To access global input source variable
#include "device_audio_task.h"

// To access DAC_BUFFER_SIZE and clear audio buffer
#include "taskAK5394A.h"
#include "usb_specific_request.h"

// I2C functions
#include "I2C.h"

/*
#include "AD7991.h"
#include "Si570.h"
#include "PCF8574.h"
#include "TMP100.h"
*/


// Low-power sleep for a number of milliseconds by means of RTC
// Use only during init, before any MCU hardware (including application use of RTC) is enabled.

void mobo_rtc_waken(volatile avr32_rtc_t *rtc, uint8_t enable) {
	while (rtc_is_busy(rtc));
	if (enable)
		rtc->ctrl |= AVR32_RTC_WAKE_EN_MASK;		// Set waken
	else
		rtc->ctrl &= ~AVR32_RTC_WAKE_EN_MASK;		// Clear waken
}

void mobo_sleep_rtc_ms(uint16_t time_ms) {
	mobo_rtc_waken(&AVR32_RTC, 0);					// Clear waken before sleeping
	rtc_init(&AVR32_RTC, RTC_OSC_RC, 0);			// RC clock at 115kHz, clear RTC, prescaler n=0 encoding freq/=2^(n+1)
	rtc_disable_interrupt(&AVR32_RTC);				// For good measure
	rtc_set_top_value(&AVR32_RTC, RTC_COUNTER_FREQ / 1000 / 2 * time_ms);	// Counter reset after time_ms ms, accounting for prescaler n=0 encoding freq/=2^(n+1)
	mobo_rtc_waken(&AVR32_RTC, 1);					// Set waken before sleeping
	rtc_enable(&AVR32_RTC);
	SLEEP(AVR32_PM_SMODE_DEEP_STOP);				// Disable all but RC clock
}


// Generic I2C single-byte read
#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC)
int8_t mobo_i2c_read (uint8_t *data, uint8_t device_address, uint8_t internal_address) {
	uint8_t dev_datar[1];
	int8_t retval = 0;
	
	dev_datar[0] = internal_address;
	
	if (xSemaphoreTake(I2C_busy, 0) == pdTRUE) {	// Re-take of taken semaphore returns false
//		print_dbg_char('[');

		// Start of blocking code

		if (twi_write_out(device_address, dev_datar, 1) == TWI_SUCCESS) {
			if (twi_read_in(device_address, dev_datar, 1) == TWI_SUCCESS) {
				*data = dev_datar[0];
			}
			else {
				retval = -1;						// I2C read fail
			}
		}
		else {
			retval = -2;							// I2C device address fail
		}
		// End of blocking code

		if( xSemaphoreGive(I2C_busy) == pdTRUE ) {
			if (retval == 0) {						// No error detected
				retval = 1;							// Everything went OK
			}
		}
		else {
			if (retval == 0) {						// No error detected
				retval = -3;						// Semaphore error
			}
		}
	} // Take OK
	else {
		if (retval == 0) {							// No error detected
			retval = -4;							// Semaphore error
		}
	} // Take not OK
	
	return retval;
}

// Generic I2C single-byte write
int8_t mobo_i2c_write (uint8_t device_address, uint8_t internal_address, uint8_t data) {
	uint8_t dev_dataw[2];
	int8_t retval = 0;
	dev_dataw[0] = internal_address;
	dev_dataw[1] = data;
	if (xSemaphoreTake(I2C_busy, 0) == pdTRUE) {	// Re-take of taken semaphore returns false

		// Start of blocking code
		if (twi_write_out(device_address, dev_dataw, 2) == TWI_SUCCESS) {
		}
		else {
			retval = -1;
		}
		// End of blocking code

		if( xSemaphoreGive(I2C_busy) == pdTRUE ) {
			if (retval == 0) {						// No error detected
				retval = 1;							// Everything went OK
			}
		}
		else {
			if (retval == 0) {						// No error detected
				retval = -3;						// Semaphore error
			}
		}
	} // Take OK
	else {
		if (retval == 0) {							// No error detected
			retval = -4;							// Semaphore error
		}
	} // Take not OK

	return retval;
}
#endif // #if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC)



// Sensors and actuators on FM ADC board
#ifdef HW_GEN_FMADC

void mobo_pcm1863_init(void) {						// Works 20230530
	#define DEVADR_PCM1863	0x4a					// Device address with MS/AD pin pulled low
	mobo_i2c_write (DEVADR_PCM1863, 0x00, 0x00);	// Accessing page.0
	mobo_i2c_write (DEVADR_PCM1863, 0x06, 0x50);	// page.0 0x06 left input 0b01010000 - 0x50 VIN1P, VIN1M
	mobo_i2c_write (DEVADR_PCM1863, 0x07, 0x50);	// page.0 0x07 right input 0b01010000 - 0x50 VIN2P, VIN2M
	mobo_i2c_write (DEVADR_PCM1863, 0x20, 0x51);	// page.0 0x20 clocking 0b01010001 - 0x51 SCK selection, sck, master, sck, sck, sck, auto clock detect, for 256xfs
	
	// For 512x operation (44.1 or 48ksps)
	// mobo_i2c_write (DEVADR_PCM1863, 0x26, 0x07);	// page.0 0x26 bit clock 0b00000111 - 0x07 for 512x at 22-24MHz, set MCLK / BCLK = 8. Keep defaults in address 0x25 and 0x26
	// Gain settings are in p.0 0x01 and 0x02
}



// Set and read gain setting
// Use channel = 1 or 2
// Use gain = 0, 1, 2, 3 to set
// Use gain = 0xFF to read
uint8_t mobo_fmadc_gain(uint8_t channel, uint8_t gain) {
	static uint8_t staticgain[2];
	
	if (channel == 1) {								// RSEL1a, RSEL1b = GPIO_04, GPIO_03 = TP50, TP51 = PX56, PX55
		if (gain == FMADC_REPORT) {
			return staticgain[0];					// Stored gain for channel 1
		}
		
		else if ( (gain >= FMADC_MINGAIN) && (gain <= FMADC_MAXGAIN) ) {
			staticgain[0] = gain;
			
			if (gain == 0) {
				gpio_clr_gpio_pin(AVR32_PIN_PX56);	// RSEL1a = 0
				gpio_clr_gpio_pin(AVR32_PIN_PX55);	// RSEL1b = 0
			}
			else if (gain == 1) {
				gpio_clr_gpio_pin(AVR32_PIN_PX56);	// RSEL1a = 0
				gpio_set_gpio_pin(AVR32_PIN_PX55);	// RSEL1b = 1
			}
			else if (gain == 2) {
				gpio_set_gpio_pin(AVR32_PIN_PX56);	// RSEL1a = 1
				gpio_clr_gpio_pin(AVR32_PIN_PX55);	// RSEL1b = 0
			}
			else if (gain == 3) {
				gpio_set_gpio_pin(AVR32_PIN_PX56);	// RSEL1a = 1
				gpio_set_gpio_pin(AVR32_PIN_PX55);	// RSEL1b = 1
			}
			return staticgain[0];					// Stored gain for channel 1
		}
		else {
			return FMADC_ERROR_G1;					// Wrong gain channel 1
		}
	} // end channel == 1
	else if (channel == 2) {						// RSEL2a, RSEL2b = GPIO_08, GPIO_05 = TP68, TP69 = PX32, PX29
		if (gain == FMADC_REPORT) {
			return staticgain[1];					// Stored gain for channel 2
		}
		
		else if ( (gain >= FMADC_MINGAIN) && (gain <= FMADC_MAXGAIN) ) {
			staticgain[1] = gain;
			
			if (gain == 0) {
				gpio_clr_gpio_pin(AVR32_PIN_PX32);	// RSEL2a = 0
				gpio_clr_gpio_pin(AVR32_PIN_PX29);	// RSEL2b = 0
			}
			else if (gain == 1) {
				gpio_clr_gpio_pin(AVR32_PIN_PX32);	// RSEL2a = 0
				gpio_set_gpio_pin(AVR32_PIN_PX29);	// RSEL2b = 1
			}
			else if (gain == 2) {
				gpio_set_gpio_pin(AVR32_PIN_PX32);	// RSEL2a = 1
				gpio_clr_gpio_pin(AVR32_PIN_PX29);	// RSEL2b = 0
			}
			else if (gain == 3) {
				gpio_set_gpio_pin(AVR32_PIN_PX32);	// RSEL2a = 1
				gpio_set_gpio_pin(AVR32_PIN_PX29);	// RSEL2b = 1
			}
			return staticgain[1];					// Stored gain for channel 2
		}
		else {
			return FMADC_ERROR_G2;					// Wrong gain channel 2
		}
	} // end channel == 2
	else {
			return FMADC_ERROR_CH;					// Wrong channel
	}
}


#endif


#ifdef HW_GEN_AB1X
	void mobo_led(uint8_t fled) {
		gpio_enable_pin_pull_up(AVR32_PIN_PA04);	// Floating: Active high. GND: Active low

		if (gpio_get_pin_value(AVR32_PIN_PA04) == 1) {	// Active high
			if (fled == FLED_DARK) {
				gpio_clr_gpio_pin(AVR32_PIN_PX29);	// Clear RED light on external AB-1.1 LED
				gpio_clr_gpio_pin(AVR32_PIN_PX32);	// Clear GREEN light on external AB-1.1 LED
			}
			else if (fled == FLED_RED) {
				gpio_set_gpio_pin(AVR32_PIN_PX29);	// Set RED light on external AB-1.1 LED
				gpio_clr_gpio_pin(AVR32_PIN_PX32);	// Clear GREEN light on external AB-1.1 LED
			}
			else if (fled == FLED_GREEN) {
				gpio_clr_gpio_pin(AVR32_PIN_PX29);	// Clear RED light on external AB-1.1 LED
				gpio_set_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED
			}
			else if (fled == FLED_YELLOW) {
				gpio_set_gpio_pin(AVR32_PIN_PX29);	// Set RED light on external AB-1.1 LED
				gpio_set_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED
			}
		}
		else {	// Active low
			if (fled == FLED_DARK) {
				gpio_set_gpio_pin(AVR32_PIN_PX29);	// Clear RED light on external AB-1.1 LED
				gpio_set_gpio_pin(AVR32_PIN_PX32);	// Clear GREEN light on external AB-1.1 LED
			}
			else if (fled == FLED_RED) {
				gpio_clr_gpio_pin(AVR32_PIN_PX29);	// Set RED light on external AB-1.1 LED
				gpio_set_gpio_pin(AVR32_PIN_PX32);	// Clear GREEN light on external AB-1.1 LED
			}
			else if (fled == FLED_GREEN) {
				gpio_set_gpio_pin(AVR32_PIN_PX29);	// Clear RED light on external AB-1.1 LED
				gpio_clr_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED
			}
			else if (fled == FLED_YELLOW) {
				gpio_clr_gpio_pin(AVR32_PIN_PX29);	// Set RED light on external AB-1.1 LED
				gpio_clr_gpio_pin(AVR32_PIN_PX32);	// Set GREEN light on external AB-1.1 LED
			}
		}
		gpio_disable_pin_pull_up(AVR32_PIN_PA04);	// Floating: Active high. GND: Active low
	}
#endif


#ifdef HW_GEN_RXMOD

// Control USB multiplexer in HW_GEN_RXMOD 
void mobo_usb_select(uint8_t usb_ch) {
	if (usb_ch == USB_CH_C) {
		gpio_clr_gpio_pin(USB_VBUS_B_PIN);				// NO USB B to MCU's VBUS pin
		gpio_clr_gpio_pin(USB_DATA_ENABLE_PIN_INV);		// Enable USB MUX
		gpio_clr_gpio_pin(USB_DATA_C0_B1_PIN);			// Select USB C to MCU's USB data pins
		gpio_set_gpio_pin(USB_VBUS_C_PIN);				// Select USB C to MCU's VBUS pin
	}
	else if (usb_ch == USB_CH_B) {
		gpio_clr_gpio_pin(USB_VBUS_C_PIN);				// NO USB C to MCU's VBUS pin
		gpio_clr_gpio_pin(USB_DATA_ENABLE_PIN_INV);		// Enable USB MUX
		gpio_set_gpio_pin(USB_DATA_C0_B1_PIN);			// Select USB B to MCU's USB data pins
		gpio_set_gpio_pin(USB_VBUS_B_PIN);				// Select USB B to MCU's VBUS pin
	}
	else {												// All vauge or undetected USB conditions
		gpio_set_gpio_pin(USB_DATA_ENABLE_PIN_INV);		// Disable USB MUX
		gpio_clr_gpio_pin(USB_VBUS_B_PIN);				// NO USB B to MCU's VBUS pin
		gpio_clr_gpio_pin(USB_VBUS_C_PIN);				// NO USB C to MCU's VBUS pin
	}
}



// Quick and dirty detect of whether front USB (C) is plugged in. No debounce here!
uint8_t mobo_usb_detect(void) {
	if (usb_ch == USB_CH_DEACTIVATE)
		return USB_CH_DEACTIVATE;						// RXMODFIX are we currently debugging what happens with USB cable detatched, with '0' ?
		
	if  (gpio_get_pin_value(AVR32_PIN_PA07) == 1)
		return USB_CH_C;


	// RXMODFIX We need a USB B detect!! Can that be done by turning VBUS_B_SEL into an input for a while? It requires the R1506=100k and R1507=200k and can only be done while VBUS_CLSEL is low
	return USB_CH_B;
}
#endif


// For the moment do nothing!
#if (defined HW_GEN_RXMOD)
void  mobo_i2s_enable(uint8_t i2s_mode) {
	if (i2s_mode == MOBO_I2S_ENABLE) {
		//		gpio_set_gpio_pin(AVR32_PIN_PX58); 					// Enable I2S data
		#ifdef USB_STATE_MACHINE_DEBUG
		//		print_dbg_char('m');								// Indicate unmute
		#endif
	}
	else if (i2s_mode == MOBO_I2S_DISABLE) {
		//		gpio_clr_gpio_pin(AVR32_PIN_PX58); 					// Disable I2S data pin
		#ifdef USB_STATE_MACHINE_DEBUG
		//		print_dbg_char('M');								// Indicate mute
		#endif
	}
}
#endif



#if (defined HW_GEN_RXMOD)

// Audio Widget HW_GEN_RXMOD LED control
void mobo_led(uint8_t fled0) {
	// red:1, green:2, blue:4
	
	if (fled0 == FLED_NO_CHG)				// No change
		return;

	if (fled0 & FLED_RED)
		gpio_clr_gpio_pin(AVR32_PIN_PA17); 	// FLED0_R
	else
		gpio_set_gpio_pin(AVR32_PIN_PA17); 	// FLED0_R
	
	if (fled0 & FLED_GREEN)
		gpio_clr_gpio_pin(AVR32_PIN_PA20); 	// FLED0_G
	else
		gpio_set_gpio_pin(AVR32_PIN_PA20); 	// FLED0_G
		
	if (fled0 & FLED_BLUE)
		gpio_clr_gpio_pin(AVR32_PIN_PA18); 	// FLED0_B
	else
		gpio_set_gpio_pin(AVR32_PIN_PA18); 	// FLED0_B
}

// Front panel RGB LED control
void mobo_led_select(U32 frequency, uint8_t source) {
	// Don't assign BLUE, future MQA implementations may crave that one. 

/*	
	static U32 prev_frequency = FREQ_INVALID;
	static uint8_t prev_source = MOBO_SRC_INVALID;
	
	if (frequency == FREQ_NOCHANGE) {
		frequency = prev_frequency;
	}
	
	// Are we changing anything in hardware? If not, just exit 
	if ( (frequency == prev_frequency) && (source == prev_source) ) {
		return;
	}

	// else, since we didn't return
	prev_source = source;
	prev_frequency = frequency;

*/

	// Source indication on single LED
	switch (source) {
		case MOBO_SRC_NONE: {
			
			#ifdef FLED_SCANNING	// Indicate scanning or fault
				mobo_led(FLED_SCANNING);
			#else
				mobo_led(FLED_WHITE);	// Indicate fault for now
			#endif
					
			// No source is indicated as USB audio
			// if (FEATURE_IMAGE_UAC1_AUDIO)
			//		source = MOBO_SRC_UAC1;
			// else if (FEATURE_IMAGE_UAC2_AUDIO)
			//		source = MOBO_SRC_UAC2;
		}
		break;

		case MOBO_SRC_UAC1:
			mobo_led(FLED_GREEN);	// Classical color UAC1
		break;

		case MOBO_SRC_UAC2:
			mobo_led(FLED_RED);		// Classical color UAC2
		break;

		case MOBO_SRC_SPDIF0:
			mobo_led(FLED_YELLOW);
		break;

		case MOBO_SRC_TOSLINK1:
			mobo_led(FLED_PURPLE);
		break;

		case MOBO_SRC_TOSLINK0:
			mobo_led(FLED_CYAN);
		break;
		
		default:
			mobo_led(FLED_DARK);	// Indicate fault for now
		break;
	}


/*

On revision C board for Boenicke, these pins are connected:
RATE_LED0 PA01
RATE_LED1 PA00
RATE_LED2 PA26
On revision A board and Henry Audio boards, pins are not connected.
We set them regardless. We could wrap them in #ifdef FEATURE_PRODUCT_BOEC1 if that were important

RATE_LED[2 1 0] = 0 0 0 44.1
RATE_LED[2 1 0] = 0 0 1 48
RATE_LED[2 1 0] = 0 1 0 88.2
RATE_LED[2 1 0] = 0 1 1 96
RATE_LED[2 1 0] = 1 0 0 176.4
RATE_LED[2 1 0] = 1 0 1 192
RATE_LED[2 1 0] = 1 1 0 TBD
RATE_LED[2 1 0] = 1 1 1 TBD

*/

	switch (frequency) {

		case FREQ_44:
			    gpio_clr_gpio_pin(AVR32_PIN_PA01);
			  gpio_clr_gpio_pin(AVR32_PIN_PA00);
			gpio_clr_gpio_pin(AVR32_PIN_PA26);
		break;

		case FREQ_48:
			    gpio_set_gpio_pin(AVR32_PIN_PA01);
			  gpio_clr_gpio_pin(AVR32_PIN_PA00);
			gpio_clr_gpio_pin(AVR32_PIN_PA26);
		break;

		case FREQ_88:
			    gpio_clr_gpio_pin(AVR32_PIN_PA01);
			  gpio_set_gpio_pin(AVR32_PIN_PA00);
			gpio_clr_gpio_pin(AVR32_PIN_PA26);
		break;

		case FREQ_96:
			    gpio_set_gpio_pin(AVR32_PIN_PA01);
			  gpio_set_gpio_pin(AVR32_PIN_PA00);
			gpio_clr_gpio_pin(AVR32_PIN_PA26);
		break;

		case FREQ_176:
			    gpio_clr_gpio_pin(AVR32_PIN_PA01);
			  gpio_clr_gpio_pin(AVR32_PIN_PA00);
			gpio_set_gpio_pin(AVR32_PIN_PA26);
		break;

		case FREQ_192:
			    gpio_set_gpio_pin(AVR32_PIN_PA01);
			  gpio_clr_gpio_pin(AVR32_PIN_PA00);
			gpio_set_gpio_pin(AVR32_PIN_PA26);
		break;
	}
}

#endif // LED for HW_GEN_RXMOD



#if (defined  HW_GEN_RXMOD)

// RXmod SPDIF mux control
void mobo_rxmod_input(uint8_t input_sel) {
	//	print_dbg_char_hex(input_sel);

	taskENTER_CRITICAL();					// Don't let OS interrupt MUX control!

	if (input_sel == MOBO_SRC_TOSLINK1) {		// Controlling MUX chip
		gpio_clr_gpio_pin(AVR32_PIN_PX03);	// SP_SEL0 = 0
		gpio_set_gpio_pin(AVR32_PIN_PX02);	// SP_SEL1 = 1
	}
	else if (input_sel == MOBO_SRC_TOSLINK0) {
		gpio_clr_gpio_pin(AVR32_PIN_PX03);	// SP_SEL0 = 0
		gpio_clr_gpio_pin(AVR32_PIN_PX02);	// SP_SEL1 = 0
	}
	else if (input_sel == MOBO_SRC_SPDIF0) {
		gpio_set_gpio_pin(AVR32_PIN_PX03);	// SP_SEL0 = 1
		gpio_set_gpio_pin(AVR32_PIN_PX02);	// SP_SEL1 = 1
	}
	else if (input_sel == MOBO_SRC_NONE) {
		gpio_set_gpio_pin(AVR32_PIN_PX03);	// SP_SEL0 = 1
		gpio_clr_gpio_pin(AVR32_PIN_PX02);	// SP_SEL1 = 0
	}
	
	taskEXIT_CRITICAL();

}

#endif // RXmod hardware controls

// Sample rate detector based on ADC LRCK polling
// You may be looking for the USB sample rate definition, Speedx_hs
uint32_t mobo_srd(void) {
	uint32_t temp;
	uint8_t freqs[6];
	uint8_t attempts = 0;
	freqs[0] = 1;					// 44.1 hits
	freqs[1] = 1;					// 48 hits
	freqs[2] = 1;					// 88.2 hits
	freqs[3] = 1;					// 96 hits
	freqs[4] = 1;					// 176.4 hits
	freqs[5] = 1;					// 196 hits

	#define SRD_MAX_ATTEMPTS	5		// How many total attempts
	#define SRD_AFE_DETECTS		3		// How many attempts to declare a safe detection?

	while (attempts++ < SRD_MAX_ATTEMPTS) {
		temp = mobo_srd_asm2();
		switch (temp) {
			case FREQ_44:
				if (freqs[0]++ >= SRD_AFE_DETECTS) {
					return FREQ_44;
				}
			break;
			case FREQ_48:
				if (freqs[1]++ >= SRD_AFE_DETECTS) {
					return FREQ_48;
				}
			break;
			case FREQ_88:
				if (freqs[2]++ >= SRD_AFE_DETECTS) {
					return FREQ_88;
				}
			break;
			case FREQ_96:
				if (freqs[3]++ >= SRD_AFE_DETECTS) {
					return FREQ_96;
				}
			break;
			case FREQ_176:
				if (freqs[4]++ >= SRD_AFE_DETECTS) {
					return FREQ_176;
				}
			break;
			case FREQ_192:
				if (freqs[5]++ >= SRD_AFE_DETECTS) {
					return FREQ_192;
				}
			break;
		}
		
	}
	
	return FREQ_TIMEOUT;

} // mobo_srd()

// Sample rate detection test
// This is MCU assembly code which replaces the non-functional sample rate detector inside the WM8804.
// It uses the same code for 44.1 and 48, and for 88.2 and 96. 176.4 and 192 are messed up too.
// The WM8804 sample rate change interrupt is based on its faulty detector and can't be trusted either.
// Todo: Make the pin to poll a parameter to the function rather than hard-coded.
//
// Compile with something like this:
// http://www.delorie.com/djgpp/v2faq/faq8_20.html
// gives:
// avr32-gcc -DFEATURE_BOARD_DEFAULT=feature_board_usbi2s -DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio -DFEATURE_IN_DEFAULT=feature_in_normal -DFEATURE_OUT_DEFAULT=feature_out_normal -DFEATURE_ADC_DEFAULT=feature_adc_none -DFEATURE_DAC_DEFAULT=feature_dac_generic -DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 -DFEATURE_LOG_DEFAULT=feature_log_500ms -DFEATURE_FILTER_DEFAULT=feature_filter_fir -DFEATURE_QUIRK_DEFAULT=feature_quirk_none -DUSB_STATE_MACHINE_DEBUG -DHW_GEN_DIN10 -DFEATURE_PRODUCT_AB1x -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -c -g -O2 -Wa,-a,-ad srd_test.c > srd_test.lst
// "/cygdrive/c/Program Files (x86)/Atmel/AVR Tools/AVR Toolchain/bin/avr32-gcc" -DFEATURE_BOARD_DEFAULT=feature_board_usbi2s -DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio -DFEATURE_IN_DEFAULT=feature_in_normal -DFEATURE_OUT_DEFAULT=feature_out_normal -DFEATURE_ADC_DEFAULT=feature_adc_none -DFEATURE_DAC_DEFAULT=feature_dac_generic -DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 -DFEATURE_LOG_DEFAULT=feature_log_500ms -DFEATURE_FILTER_DEFAULT=feature_filter_fir -DFEATURE_QUIRK_DEFAULT=feature_quirk_none -DUSB_STATE_MACHINE_DEBUG -DHW_GEN_DIN10 -DFEATURE_PRODUCT_AB1x -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -c -g -O2 -Wa,-a,-ad srd_test.c > srd_test.lst
//
// Alternatively:
// avr32-gcc -DFEATURE_BOARD_DEFAULT=feature_board_usbi2s -DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio -DFEATURE_IN_DEFAULT=feature_in_normal -DFEATURE_OUT_DEFAULT=feature_out_normal -DFEATURE_ADC_DEFAULT=feature_adc_none -DFEATURE_DAC_DEFAULT=feature_dac_generic -DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 -DFEATURE_LOG_DEFAULT=feature_log_500ms -DFEATURE_FILTER_DEFAULT=feature_filter_fir -DFEATURE_QUIRK_DEFAULT=feature_quirk_none -DUSB_STATE_MACHINE_DEBUG -DHW_GEN_DIN10 -DFEATURE_PRODUCT_AB1x -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -S -fverbose-asm -g -O2 srd_test.c
// avr32-gcc -DFEATURE_BOARD_DEFAULT=feature_board_usbi2s -DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio -DFEATURE_IN_DEFAULT=feature_in_normal -DFEATURE_OUT_DEFAULT=feature_out_normal -DFEATURE_ADC_DEFAULT=feature_adc_none -DFEATURE_DAC_DEFAULT=feature_dac_generic -DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 -DFEATURE_LOG_DEFAULT=feature_log_500ms -DFEATURE_FILTER_DEFAULT=feature_filter_fir -DFEATURE_QUIRK_DEFAULT=feature_quirk_none -DUSB_STATE_MACHINE_DEBUG -DHW_GEN_DIN10 -DFEATURE_PRODUCT_AB1x -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -S -g -O2 srd_test.c
//
// A good asm syntax list:
// http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
//
// Test code for learning the asm code:
/*
#include "gpio.h"
#include <avr32/io.h>
#include "compiler.h"
#define GPIO  AVR32_GPIO
int foo(void) {
	#define TIMEOUT_LIM 8000;
	int timeout = 8000;
	// Code to determine GPIO constants, rewrite this (2 positions!) first, compile this section, then modify asm
	volatile avr32_gpio_port_t *gpio_port = &GPIO.port[AVR32_PIN_PA04 >> 5];
	while ( (timeout != 0) && ( ((gpio_port->pvr >> (AVR32_PIN_PA04 & 0x1F)) & 1) == 0) ) {
		timeout --;
	}
	return timeout;
}
*/
uint32_t mobo_srd_asm2(void) {
	uint32_t timeout;

	// see srd_test03.c and srd_test03.lst

	// HW_GEN_RXMOD: Moved from PX09, pin 49 to PA05, pin 124, to PX36, pin 44 same as used by other code

	// Recompile prototype c to change io pin!
	// Test is done for up to 1 half period, then 2 full periods

	gpio_enable_gpio_pin(AVR32_PIN_PX36);	// Enable GPIO pin, not special IO (also for input). Needed?
	
	// PA05 is GPIO. PX26 and PX36 are special purpose clock pins

	asm volatile(
		//		"ssrf	16				\n\t"	// Disable global interrupt
		"mov	%0, 	2000	\n\t"	// Load timeout
		"mov	r9,		-60928	\n\t"	// Immediate load, set up pointer to PX36, recompile C for other IO pin, do once

		// If bit is 0, branch to loop while 0. If bit was 1, continue to loop while 1
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	S3				\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)

		// Wait while bit is 1, then count two half periods
		"S0:					\n\t"	// Loop while PX36 is 1
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	S0_done			\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S0				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S0_done:				\n\t"

		"mfsr	r10, 264		\n\t"	// Load 1st cycle counter into r10

		"S1:					\n\t"	// Loop while PX36 is 0
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	S1_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S1				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S1_done:				\n\t"

		"S2:					\n\t"	// Loop while PX36 is 1
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	S2_done			\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S2				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S2_done:				\n\t"
		"rjmp	SRETURN__		\n\t"



		// Wait while bit is 0, then count two half periods
		"S3:					\n\t"	// Loop while PX36 is 0
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	S3_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S3				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S3_done:				\n\t"

		"mfsr	r10, 264		\n\t"	// Load 1st cycle counter into r10

		"S4:					\n\t"	// Loop while PX36 is 1
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	S4_done			\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S4				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S4_done:				\n\t"

		"S5:					\n\t"	// Loop while PX36 is 0
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	S5_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S5				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S5_done:				\n\t"
		"rjmp	SRETURN__		\n\t"


		"SRETURN__:				\n\t"

		"mfsr	%0, 264			\n\t"	// Load 2nd cycle counter into r11
		"sub	%0, r10			\n\t"	// Return difference from 1st to 2nd cycle counter


		"SCOUNTD:				\n\t"	// Countdown reached, %0 is 0

		//		"csrf	16				\n\t"	// Enable global interrupt
		:	"=r" (timeout)				// One output register
		:								// No input registers
		:	"r8", "r9", "r10"			// Clobber registers, pushed/popped unless assigned by GCC as temps
	);


// 	timeout = 150 - timeout;

	// It looks like we have approx. With 66MHz CPU clock it looks like 1 ticks
	// Results from measurements and math:
	//  44.1 1478-1580 (1496.6)
	//  48.0 1358-1452 (1375.0)
	//	88.2  739- 790 ( 748.3)
	//  96.0  679- 726 ( 687.5)
	// 176.4  369- 396 ( 374.2)
	// 192.0  339- 363 ( 343.8)

	#define SLIM_44_LOW		1478
	#define SLIM_44_HIGH	1580 		// Gives timeout of 2000
	#define SLIM_48_LOW		1358
	#define SLIM_48_HIGH	1452
	#define SLIM_88_LOW		739
	#define SLIM_88_HIGH	790
	#define SLIM_96_LOW		679
	#define SLIM_96_HIGH	726
	#define SLIM_176_LOW	369			// Add margin??
	#define SLIM_176_HIGH	396
	#define SLIM_192_LOW	339
	#define SLIM_192_HIGH	367			// Analysis saw up to 366
	
	// Limits range from 0x0153 to 0x062C. If timeout & 0x0000F000 isn't 0 then something went wrong and result should be ignored
	
	if ( (timeout >= SLIM_44_LOW) && (timeout <= SLIM_44_HIGH) ) {
//		print_dbg_char('1');
		return FREQ_44;
	}
	if ( (timeout >= SLIM_48_LOW) && (timeout <= SLIM_48_HIGH) ) {
//		print_dbg_char('2');
		return FREQ_48;
	}
	if ( (timeout >= SLIM_88_LOW) && (timeout <= SLIM_88_HIGH) ) {
//		print_dbg_char('3');
		return FREQ_88;
	}
	if ( (timeout >= SLIM_96_LOW) && (timeout <= SLIM_96_HIGH) ) {
//		print_dbg_char('4');
		return FREQ_96;
	}
	if ( (timeout >= SLIM_176_LOW) && (timeout <= SLIM_176_HIGH) ) {
//		print_dbg_char('5');
		return FREQ_176;
	}
	if ( (timeout >= SLIM_192_LOW) && (timeout <= SLIM_192_HIGH) ) {
//		print_dbg_char('6');
		return FREQ_192;
	}
	if (timeout & 0x0000F000) {		// According to tests done. This may be the signature of the RTOS
//		print_dbg_char('I');
		return FREQ_INVALID;
	}
		
	else {
//		print_dbg_char('F');
		return FREQ_TIMEOUT;	// Every uncertainty treated as timeout...
	}

} // mobo_srd_asm2()



// Wait for N half-periods of LRCK
// Seems to work for WM8804 input but channels are swapped and one is delayed for 1 sample for PCM1863. Why? What is the programming context? Do all clocks run as they are supposed to?
uint32_t mobo_wait_LRCK_asm(void) {
	uint32_t timeout;

	// see srd_test03.c and srd_test03.lst

	// HW_GEN_RXMOD: Moved from PX09, pin 49 to PA05, pin 124, to PX36, pin 44 same as used by other code

	// Recompile prototype c to change io pin!
	// Test is done for up to 1 half period, then 2 full periods

	gpio_enable_gpio_pin(AVR32_PIN_PX36);	// Enable GPIO pin, not special IO (also for input). Needed?
	
	// PA05 is GPIO. PX26 and PX36 are special purpose clock pins

	asm volatile(
	//		"ssrf	16				\n\t"	// Disable global interrupt
	"mov	%0, 	500		\n\t"	// Load timeout
	"mov	r9,		-60928	\n\t"	// Immediate load, set up pointer to PX36, recompile C for other IO pin, do once

	"S0x:					\n\t"	// Loop while PX36 is 1
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"brne	S0x_done		\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S0x				\n\t"	// Not done counting down
	"rjmp	SCOUNTDx		\n\t"	// Countdown reached
	"S0x_done:				\n\t"

	"S1x:					\n\t"	// Loop while PX36 is 0
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"breq	S1x_done		\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S1x				\n\t"	// Not done counting down
	"rjmp	SCOUNTDx		\n\t"	// Countdown reached
	"S1x_done:				\n\t"

	"S2x:					\n\t"	// Loop while PX36 is 1
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"brne	S2x_done		\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S2x				\n\t"	// Not done counting down
	"rjmp	SCOUNTDx		\n\t"	// Countdown reached
	"S2x_done:				\n\t"

/*
	"S3x:					\n\t"	// Loop while PX36 is 0
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"breq	S3x_done		\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S3x				\n\t"	// Not done counting down
	"rjmp	SCOUNTDx		\n\t"	// Countdown reached
	"S3x_done:				\n\t"
*/

	"SCOUNTDx:				\n\t"	// Countdown reached, %0 is 0

	//		"csrf	16				\n\t"	// Enable global interrupt
	:	"=r" (timeout)				// One output register
	:								// No input registers
	:	"r8", "r9"					// Clobber registers, pushed/popped unless assigned by GCC as temps
	);

	return timeout;

} // mobo_wait_LRCK_asm




#ifdef HW_GEN_RXMOD
// Handle spdif and toslink input
void mobo_handle_spdif(uint8_t width) {
	static int ADC_buf_DMA_write_prev = INIT_ADC_I2S;
	int ADC_buf_DMA_write_temp = 0;
	static U32 spk_index = 0;
	static S16 gap = DAC_BUFFER_SIZE;
	S16 old_gap = DAC_BUFFER_SIZE;
	static S16 iterations = 0;

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


// The Henry Audio and QNKTC series of hardware only use NORMAL I2S with left before right æææ move this to some .h file!!
#if (defined HW_GEN_AB1X) || (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC)
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
	if ( (ADC_buf_DMA_write_prev == INIT_ADC_I2S)	|| (ADC_buf_I2S_IN == INIT_ADC_I2S) )	 {	// Do the init on synchronous sampling ref. ADC DMA timing

		print_dbg_char('Y');	// I2S OUT consumer starting up

		// Clear incoming SPDIF before enabling pdca to keep filling it
		mobo_clear_adc_channel();

		ADC_buf_DMA_write_prev = ADC_buf_DMA_write_temp;
		ADC_buf_I2S_IN = INIT_ADC_I2S_st2;	// Move on to init stage 2
	}

	if (spdif_rx_status.reliable == 0) { // Temporarily unreliable counts as silent and halts processing
		spdif_rx_status.silent = 1;
		ADC_buf_DMA_write_prev = ADC_buf_DMA_write_temp;			// Respond as soon as .reliable is set
	}

	// Has producer's buffer been toggled by interrupt driven DMA code?
	// If so, check it for silence. If selected as source, copy all of producer's data. (And perform skip/insert.)
	// Only bother if .reliable != 0
	else if (spdif_rx_status.buffered == 0) {
		spdif_rx_status.silent = 0;
	}
	else if (ADC_buf_DMA_write_temp != ADC_buf_DMA_write_prev) { // Check if producer has sent more data
		ADC_buf_DMA_write_prev = ADC_buf_DMA_write_temp;

		if (input_select == MOBO_SRC_NONE)
			iterations = 0;
		else if ( ( (input_select == MOBO_SRC_TOSLINK0) || (input_select == MOBO_SRC_TOSLINK1) || (input_select == MOBO_SRC_SPDIF0) ) ) {
			if (iterations < 100)
				iterations++;
		}

		// Silence / DC detector 2.0
//		if (spdif_rx_status.reliable == 1) {			// This code is unable to detect silence in a shut-down WM8805
		for (i=0 ; i < ADC_BUFFER_SIZE ; i++) {
			if (ADC_buf_DMA_write_temp == 0)	// End as soon as a difference is spotted
				sample_temp = audio_buffer_0[i] & 0x00FFFF00;
			else if (ADC_buf_DMA_write_temp == 1)
				sample_temp = audio_buffer_1[i] & 0x00FFFF00;

			if ( (sample_temp != 0x00000000) && (sample_temp != 0x00FFFF00) ) // "zero" according to tested sources
				i = ADC_BUFFER_SIZE + 10;
		}

		if (i >= ADC_BUFFER_SIZE + 10) {		// Non-silence was detected
			spdif_rx_status.silent = 0;
		}
		else {									// Silence was detected, update flag to SPDIF RX code
			spdif_rx_status.silent = 1;
		}
//		}

		if ( ( (input_select == MOBO_SRC_TOSLINK0) || (input_select == MOBO_SRC_TOSLINK1) || (input_select == MOBO_SRC_SPDIF0) ) ) {

			// Startup condition: must initiate consumer's write pointer to where-ever its read pointer may be
			if (ADC_buf_I2S_IN == INIT_ADC_I2S_st2) {

				ADC_buf_I2S_IN = ADC_buf_DMA_write_temp;	// Disable further init, select correct audio_buffer_0/1
				dac_must_clear = DAC_READY;					// Prepare to send actual data to DAC interface

				// USB code has !0 detection, semaphore checks etc. etc. around here. See line 744 in uac2_dat.c
//				skip_enable = 0;
				gap = DAC_BUFFER_SIZE; // Ideal gap value

				// New co-sample verification routine
				DAC_buf_DMA_read_local = DAC_buf_DMA_read;
				num_remaining = spk_pdca_channel->tcr;
				// Did an interrupt strike just there? Check if DAC_buf_DMA_read is valid. If not valid, interrupt won't strike again
				// for a long time. In which we simply read the counter again
				if (DAC_buf_DMA_read_local != DAC_buf_DMA_read) {
					DAC_buf_DMA_read_local = DAC_buf_DMA_read;
					num_remaining = spk_pdca_channel->tcr;
				}
				DAC_buf_OUT = DAC_buf_DMA_read_local;

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

			if (DAC_buf_OUT != DAC_buf_DMA_read_local) { 	// DAC DMA and seq. code using same buffer
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

			if (iterations > 20) {
				// Apply skip/insert
				// Just don't do it right after starting playback
				if ((gap < old_gap) && (gap < SPK_GAP_L3)) {
					samples_to_transfer_OUT = 0;		// Do some skippin'
#ifdef USB_STATE_MACHINE_DEBUG
 					print_dbg_char('s');
#endif
				}
				else if ((gap > old_gap) && (gap > SPK_GAP_U3)) {
					samples_to_transfer_OUT = 2;		// Do some insertin'
#ifdef USB_STATE_MACHINE_DEBUG
 					print_dbg_char('i');
#endif
				}

				// Are we about to loose skip/insert targets? If so, revert to RX's MCLK and run synchronous from now on
				if ( (gap <= SPK_GAP_LX) || (gap >= SPK_GAP_UX) ) {
					// Explicitly enable receiver's MCLK generator? ADC_site
					mobo_xo_select(FREQ_RXNATIVE, input_select);
#ifdef USB_STATE_MACHINE_DEBUG
					print_dbg_char('X');
#endif
				}
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

			// Prepare to copy all of producer's most recent data to consumer's buffer
			if (ADC_buf_DMA_write_temp == 1)
				gpio_set_gpio_pin(AVR32_PIN_PX18);			// Pin 84
			else if (ADC_buf_DMA_write_temp == 0)
				gpio_clr_gpio_pin(AVR32_PIN_PX18);			// Pin 84

			// Apply megaskip when DC or zero is detected
			if (spdif_rx_status.silent == 1) {				// Silence was detected
				if (gap < SPK_GAP_LM ) {					// Are we close or past the limit for having to skip?
					megaskip = SPK_GAP_UD - gap;			// This is as far as we can safely skip, one ADC package at a time
				}
				else if (gap > SPK_GAP_UM ) {				// Are we close to or past the limit for having to insert?
					megaskip = gap - SPK_GAP_LD; 			// This is as far as we can safely insert, one ADC package at a time
				}
			}
			else {
				megaskip = 0;	// Not zero -> no big skips!
			}


			// We're skipping or about to skip. In case of silence, do a good and proper skip by copying nothing
			if (megaskip >= ADC_BUFFER_SIZE) {

				// Use crystal oscillator. It's OK to call this repeatedly even if XO wasn't disabled
				mobo_xo_select(spdif_rx_status.frequency, input_select);

#ifdef USB_STATE_MACHINE_DEBUG
				print_dbg_char('S');
#endif
				samples_to_transfer_OUT = 1; 	// Revert to default:1. I.e. only one skip or insert in next ADC package
				megaskip -= ADC_BUFFER_SIZE;	// We have jumped over one whole ADC package
				// FIX: Is there a need to null the buffers and avoid re-use of old DAC buffer content?
			}
			// We're inserting or about to insert. In case of silence, do a good and proper insert by doubling an ADC package
			else if (megaskip <= -ADC_BUFFER_SIZE) {

				// Use crystal oscillator. It's OK to call this repeatedly even if XO wasn't disabled
				mobo_xo_select(spdif_rx_status.frequency, input_select);

#ifdef USB_STATE_MACHINE_DEBUG
				print_dbg_char('I');
#endif
				samples_to_transfer_OUT = 1; // Revert to default:1. I.e. only one skip or insert per USB package
				megaskip += ADC_BUFFER_SIZE;	// Prepare to -insert- one ADC package, i.e. copying two ADC packages

				for (i=0 ; i < ADC_BUFFER_SIZE *2 ; i+=2) { // Mind the *2
					if (dac_must_clear == DAC_READY) {
						if (DAC_buf_OUT == 0) {
							spk_buffer_0[spk_index+OUT_LEFT] = 0; 
							spk_buffer_0[spk_index+OUT_RIGHT] = 0;
						}
						else if (DAC_buf_OUT == 1) {
							spk_buffer_1[spk_index+OUT_LEFT] = 0;
							spk_buffer_1[spk_index+OUT_RIGHT] = 0;
						}
					}

					spk_index += 2;							// æææ Analyze this code again, what is actually going on with the megaskips?
					if (spk_index >= DAC_BUFFER_SIZE) {
						spk_index -= DAC_BUFFER_SIZE;
						DAC_buf_OUT = 1 - DAC_buf_OUT;
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
							if (DAC_buf_OUT == 0) {
								spk_buffer_0[spk_index+OUT_LEFT] = sample_L;
								spk_buffer_0[spk_index+OUT_RIGHT] = sample_R;
							}
							else if (DAC_buf_OUT == 1) {
								spk_buffer_1[spk_index+OUT_LEFT] = sample_L;
								spk_buffer_1[spk_index+OUT_RIGHT] = sample_R;
							}
						}

						spk_index += 2;
						if (spk_index >= DAC_BUFFER_SIZE) {
							spk_index -= DAC_BUFFER_SIZE;
							DAC_buf_OUT = 1 - DAC_buf_OUT;

#ifdef USB_STATE_MACHINE_DEBUG
							if (DAC_buf_OUT == 1)
								gpio_set_gpio_pin(AVR32_PIN_PX30);
							else
								gpio_clr_gpio_pin(AVR32_PIN_PX30);
#endif
						}
					}
					samples_to_transfer_OUT = 1; // Revert to default:1. I.e. only one skip or insert per USB package
				} // for ADC_BUFFER_SIZE
			} // Normal operation

		} // ADC_buf_DMA_write toggle
		
//		ææææ split the toggle action into multiple rounds of the spdif handler!!
//		or make it an interruptable task or something....
		
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

	if ( (frequency != prev_frequency) || (prev_frequency == FREQ_INVALID) ) { 	// Only run at startup or when things change
	#if (defined HW_GEN_AB1X)
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
			default: // same as 44.1
				if (FEATURE_BOARD_USBI2S)
					gpio_clr_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 22.5792MHz/2 for AB-1
				else if (FEATURE_BOARD_USBDAC)
					gpio_clr_gpio_pin(AVR32_PIN_PX51);
				gpio_clr_gpio_pin(SAMPLEFREQ_VAL0);
				gpio_clr_gpio_pin(SAMPLEFREQ_VAL1);
			break;
		} // switch

	#elif (defined HW_GEN_RXMOD) 

		if (spdif_rx_status.buffered == 0) { // NB will be discontinued in hardware!
			// Old version with I2S mux
			// FIX: correlate with mode currently selected by user or auto, that's a global variable!
			if ( (source == MOBO_SRC_UAC1) || (source == MOBO_SRC_UAC2) || (source == MOBO_SRC_NONE) ) {
					gpio_set_gpio_pin(AVR32_PIN_PC01); 	// SEL_USBP_RXN = 1 defaults to USB

				// Clock source control - Permit multiple clocks to briefly short rather than risk the clock being off
				if ( (frequency == FREQ_44) || (frequency == FREQ_88) || (frequency == FREQ_176) ) {
					gpio_set_gpio_pin(AVR32_PIN_PA23); 	// 44.1 control
					gpio_clr_gpio_pin(AVR32_PIN_PA21); 	// 48 control
					gpio_clr_gpio_pin(AVR32_PIN_PX22); 	// Disable RX recovered MCLK
				}
				else {
					gpio_set_gpio_pin(AVR32_PIN_PA21); 	// 48 control
					gpio_clr_gpio_pin(AVR32_PIN_PA23); 	// 44.1 control
					gpio_clr_gpio_pin(AVR32_PIN_PX22); 	// Disable RX recovered MCLK
				}
			}
			else if ( (source == MOBO_SRC_SPDIF0) || (source == MOBO_SRC_TOSLINK1)  || (source == MOBO_SRC_TOSLINK0) ) {
				gpio_clr_gpio_pin(AVR32_PIN_PC01); 		// SEL_USBP_RXN = 0 defaults to RX-I2S with digital inputs because spdif_rx_status.buffered == 0
				gpio_set_gpio_pin(AVR32_PIN_PX22); 		// Enable RX recovered MCLK
				gpio_clr_gpio_pin(AVR32_PIN_PA23); 		// Disable XOs 44.1 control
				gpio_clr_gpio_pin(AVR32_PIN_PA21); 		// Disable XOs 48 control
			}
		}
		else { // Furrered, spdif_rx_status.buffered != 0
			// New version, possibly without I2S mux, with buffering via MCU's ADC interface
			// RXMODFIX verify ADC vs. mux! What is the test code for this?
			gpio_set_gpio_pin(AVR32_PIN_PC01); 			// SEL_USBP_RXN = 1 defaults to USB and buffering via MCU FIFO

			// Clock source control
			if (frequency == FREQ_RXNATIVE) {			// Use MCLK from SPDIF RX
				// Explicitly turn on MCLK generation in SPDIF RX?
				gpio_set_gpio_pin(AVR32_PIN_PX22); 		// Enable RX recovered MCLK
				gpio_clr_gpio_pin(AVR32_PIN_PA23); 		// 44.1 control
				gpio_clr_gpio_pin(AVR32_PIN_PA21); 		// 48 control
//				gpio_clr_gpio_pin(AVR32_PIN_PC01); 		// SEL_USBP_RXN = 0 defaults to RX-I2S Don't bypass with MUX when it is buffered!
			}
			else if ( (frequency == FREQ_44) || (frequency == FREQ_88) || (frequency == FREQ_176) ) {
				gpio_set_gpio_pin(AVR32_PIN_PA23); 		// 44.1 control
				gpio_clr_gpio_pin(AVR32_PIN_PA21); 		// 48 control
				gpio_clr_gpio_pin(AVR32_PIN_PX22); 		// Disable RX recovered MCLK
			}
			else { // MOBO_SRC_NONE defaults to 48kHz domain? Is that consistent in code?
				gpio_set_gpio_pin(AVR32_PIN_PA21); 		// 48 control
				gpio_clr_gpio_pin(AVR32_PIN_PA23); 		// 44.1 control
				gpio_clr_gpio_pin(AVR32_PIN_PX22); 		// Disable RX recovered MCLK
			}
			
			#ifdef USB_REDUCED_DEBUG
				// Report to CPU (when present)
				switch (frequency) {
					case FREQ_44:
						print_cpu_char(CPU_CHAR_44); // Inform CPU (when present)
					break;
					case FREQ_48:
						print_cpu_char(CPU_CHAR_48); // Inform CPU (when present)
					break;
					case FREQ_88:
						print_cpu_char(CPU_CHAR_88); // Inform CPU (when present)
					break;
					case FREQ_96:
						print_cpu_char(CPU_CHAR_96); // Inform CPU (when present)
					break;
					case FREQ_176:
						print_cpu_char(CPU_CHAR_176); // Inform CPU (when present)
					break;
					case FREQ_192:
						print_cpu_char(CPU_CHAR_192); // Inform CPU (when present)
					break;
					case FREQ_RXNATIVE:
						print_cpu_char(CPU_CHAR_REGEN); // Inform CPU (when present)
					break;
					default:
						print_cpu_char(CPU_CHAR_RATE_DEF); // Inform CPU (when present)
					break;
				}			
			#endif
		}
	#elif (defined HW_GEN_FMADC)
		// FMADC_site 
		// 48ksps domain is permanently turned on
	#else
		#error undefined hardware
	#endif

		prev_frequency = frequency;
	} // if (frequency != prev_frequency)
}


// Master clock to DAC's I2S port frequency setup
void mobo_clock_division(U32 frequency) {

	static U32 prev_frequency = FREQ_INVALID;

	if ( (frequency != prev_frequency) || (prev_frequency == FREQ_INVALID) ) { 	// Only run at startup or when things change
		#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC)
			// RXMODFIX implement clock division and config pin
		#else
			gpio_enable_pin_pull_up(AVR32_PIN_PA03);	// Floating: stock AW with external /2. GND: modded AW with no ext. /2
		#endif
	
		pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

		#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC)		// RXMODFIX implement clock division and config pin
			if (0) {
		#else
			// External /2 variety, unmodded hardware with floating, pulled-up PA03 interpreted as 1
			if (gpio_get_pin_value(AVR32_PIN_PA03) == 1) {
		#endif
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
					default : // Treated as 44.1
						pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
									0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
									1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
									1,                  // diven - enabled
									1);                 // divided by 4
					break;
				}
			}

		// No external /2 variety, modded hardware with resistor tying PA03 to 0
		else {	// HW_GEN_RXMOD and HW_GEN_FMADC only follow this branch
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
				default :		// Treated as 44.1
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								3);                 // divided by 8
				break;
			}
		}

		gpio_disable_pin_pull_up(AVR32_PIN_PA03);	// Floating: stock AW with external /2. GND: modded AW with no ext. /2

		pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

		AK5394A_pdca_tx_enable(frequency);			// LRCK inversion will occur with FREQ_INVALID

		prev_frequency = frequency;
	}
}


// Empty the contents of the incoming pdca buffers
void mobo_clear_adc_channel(void) {
	int i;

//	gpio_set_gpio_pin(AVR32_PIN_PX18); // ch2

	for (i = 0; i < ADC_BUFFER_SIZE; i++) {
		audio_buffer_0[i] = 0;
		audio_buffer_1[i] = 0;
	}

//	gpio_clr_gpio_pin(AVR32_PIN_PX18); // ch2
}


// Empty the contents of the outgoing pdca buffers
void mobo_clear_dac_channel(void) {
	int i;

#ifdef USB_STATE_MACHINE_DEBUG
//	print_dbg_char('C');
#endif

//	gpio_set_gpio_pin(AVR32_PIN_PX17); // ch3

	for (i = 0; i < DAC_BUFFER_SIZE; i++) {
		spk_buffer_0[i] = 0;
		spk_buffer_1[i] = 0;
	}

//	gpio_clr_gpio_pin(AVR32_PIN_PX17); // ch3
}




//
//-----------------------------------------------------------------------------
// The below structure contains a number of implementation dependent definitions (user tweak stuff)
//-----------------------------------------------------------------------------
//

#define ENC_PULSES			512		// hack while removing rotary_encoder
#define AD5301_I2C_ADDRESS 1		// hack while removing AD5301

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
