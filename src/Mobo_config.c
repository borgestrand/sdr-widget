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

// Timer/counter control
#include "tc.h"

// Real-time counter management
#include "rtc.h"

// To access global input source variable
#include "device_audio_task.h"

// To access DAC_BUFFER_SIZE and clear audio buffer
#include "taskAK5394A.h"
#include "usb_specific_request.h"

// I2C functions
#include "I2C.h"



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
		return FREQ_44;
	}
	if ( (timeout >= SLIM_48_LOW) && (timeout <= SLIM_48_HIGH) ) {
		return FREQ_48;
	}
	if ( (timeout >= SLIM_88_LOW) && (timeout <= SLIM_88_HIGH) ) {
		return FREQ_88;
	}
	if ( (timeout >= SLIM_96_LOW) && (timeout <= SLIM_96_HIGH) ) {
		return FREQ_96;
	}
	if ( (timeout >= SLIM_176_LOW) && (timeout <= SLIM_176_HIGH) ) {
		return FREQ_176;
	}
	if ( (timeout >= SLIM_192_LOW) && (timeout <= SLIM_192_HIGH) ) {
		return FREQ_192;
	}
	if (timeout & 0x0000F000) {		// According to tests done. This may be the signature of the RTOS
		return FREQ_INVALID;
	}
		
	else {
		return FREQ_TIMEOUT;	// Every uncertainty treated as timeout...
	}

} // mobo_srd_asm2()



// Wait for N half-periods of LRCK on the RX side
// Seems to work for WM8804 input but channels are swapped and one is delayed for 1 sample for PCM1863. Why? What is the programming context? Do all clocks run as they are supposed to?
uint32_t mobo_wait_LRCK_RX_asm(void) {
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

/*
	"S0x:					\n\t"	// Loop while PX36 is 1
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"brne	S0x_done		\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S0x				\n\t"	// Not done counting down
	"rjmp	SCOUNTD_RX		\n\t"	// Countdown reached
	"S0x_done:				\n\t"
*/

	"S1x:					\n\t"	// Loop while PX36 is 0
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"breq	S1x_done		\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S1x				\n\t"	// Not done counting down
	"rjmp	SCOUNTD_RX		\n\t"	// Countdown reached
	"S1x_done:				\n\t"

	"S2x:					\n\t"	// Loop while PX36 is 1
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"brne	S2x_done		\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S2x				\n\t"	// Not done counting down
	"rjmp	SCOUNTD_RX		\n\t"	// Countdown reached
	"S2x_done:				\n\t"

	"S3x:					\n\t"	// Loop while PX36 is 0
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX36 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	23		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"breq	S3x_done		\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S3x				\n\t"	// Not done counting down
	"rjmp	SCOUNTD_RX		\n\t"	// Countdown reached
	"S3x_done:				\n\t"

	"SCOUNTD_RX:			\n\t"	// Countdown reached, %0 is 0

	//		"csrf	16				\n\t"	// Enable global interrupt
	:	"=r" (timeout)				// One output register
	:								// No input registers
	:	"r8", "r9"					// Clobber registers, pushed/popped unless assigned by GCC as temps
	);

	return timeout;

} // mobo_wait_LRCK_RX_asm


// Wait for N half-periods of LRCK on the TX side
uint32_t mobo_wait_LRCK_TX_asm(void) {
	uint32_t timeout;

	// see srd_test03.c and srd_test03.lst

	// HW_GEN_RXMOD: Moved from PX09, pin 49 to PA05, pin 124, to PX36, pin 44 same as used by other code

	// Recompile prototype c to change io pin!
	// Test is done for up to 1 half period, then 2 full periods

//	gpio_enable_gpio_pin(AVR32_PIN_PX27);	// DON'T! Enable GPIO pin, not special IO (also for input). Needed?
	
	// PA05 is GPIO. PX26 and PX36 are special purpose clock pins

	asm volatile(
	//		"ssrf	16				\n\t"	// Disable global interrupt
	"mov	%0, 	500		\n\t"	// Load timeout
	"mov	r9,		-60928	\n\t"	// Immediate load, set up pointer to PX27, recompile C for other IO pin, do once

/*
	"S10x:					\n\t"	// Loop while PX27 is 1
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX27 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	14		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"brne	S10x_done		\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S10x			\n\t"	// Not done counting down
	"rjmp	SCOUNTD_TX		\n\t"	// Countdown reached
	"S10x_done:				\n\t"
*/

	"S11x:					\n\t"	// Loop while PX27 is 0
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX27 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	14		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"breq	S11x_done		\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S11x			\n\t"	// Not done counting down
	"rjmp	SCOUNTD_TX		\n\t"	// Countdown reached
	"S11x_done:				\n\t"

	"S12x:					\n\t"	// Loop while PX27 is 1
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX27 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	14		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"brne	S12x_done		\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S12x			\n\t"	// Not done counting down
	"rjmp	SCOUNTD_TX		\n\t"	// Countdown reached
	"S12x_done:				\n\t"

	"S13x:					\n\t"	// Loop while PX27 is 0
	"ld.w	r8, 	r9[96]	\n\t"	// Load PX27 (and surroundings?) into r8, 		recompile C for other IO pin
	"bld	r8, 	14		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
	"breq	S13x_done		\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
	"sub	%0,	1			\n\t"	// Count down
	"brne	S13x			\n\t"	// Not done counting down
	"rjmp	SCOUNTD_TX		\n\t"	// Countdown reached
	"S13x_done:				\n\t"

	"SCOUNTD_TX:			\n\t"	// Countdown reached, %0 is 0

	//		"csrf	16				\n\t"	// Enable global interrupt
	:	"=r" (timeout)				// One output register
	:								// No input registers
	:	"r8", "r9"					// Clobber registers, pushed/popped unless assigned by GCC as temps
	);

	return timeout;

} // mobo_wait_LRCK_TX_asm




#ifdef HW_GEN_RXMOD

// Convert from pdca report to buffer address. _pos always points to left sample in LR stereo pair!
void mobo_ADC_position(U32 *last_pos, int *last_buf, U32 num_remaining, int buf) {
	*last_pos = (ADC_BUFFER_SIZE - num_remaining) & ~((U32)1); // Counting mono samples. Clearing LSB = indicate the last written left sample in L/R pair
	// Are we operating from 0 to < ADC_BUFFER_SIZE? That is safe, record the position and buffer last written to
	if (*last_pos < ADC_BUFFER_SIZE) {
		*last_buf = buf;
	}
	// Did timer_captured_ADC_buf_DMA_write count up to or beyond ADC_BUFFER_SIZE? If so, don't overflow but record the last position of the previous buffer
	// Tests indicate timer_captured_ADC_buf_DMA_write in the range 0..ADC_BUFFER_SIZE-1, and in extremely rare situations 0..ADC_BUFFER_SIZE
	else {
		num_remaining = max(num_remaining - 2, ADC_BUFFER_SIZE - 2); // Move back one stereo sample or more. Rather risk deleting samples than overflowing buffer access
		*last_pos = (ADC_BUFFER_SIZE - num_remaining) & ~((U32)1); // Counting mono samples. Clearing LSB = start with left sample
		buf = 1  - buf;
		*last_buf = buf;
	}
}




// Handle spdif and toslink input
void mobo_handle_spdif(U32 *si_index_low, S32 *si_score_high, U32 *si_index_high, S32 *cache_L, S32 *cache_R, S32 *num_samples, Bool *cache_holds_silence) {
	
	
// Overwritten parameters, rewrite without '*' if handled inline instead of as function call
	static int prev_ADC_buf_DMA_write = INIT_ADC_I2S;
	int local_ADC_buf_DMA_write = 0;
	static U32 spk_index = 0;

	U32 i;								// Generic counter

	U8 local_DAC_buf_DMA_read;			// Local copy read in atomic operations
	U32 num_remaining;

	S32 sample_temp = 0;
	S32 sample_L = 0;
	S32 sample_R = 0;

// Reused variables from uac2_dat
	static S32 prev_sample_L = 0;	// Enable delayed writing to cache, initiated to 0, new value survives to next iteration
	static S32 prev_sample_R = 0;
	S32 diff_value = 0;
	S32 diff_sum = 0;
	S32 si_score_low = 0x7FFFFFFF;
	static S32 prev_diff_value = 0;	// Initiated to 0, new value survives to next iteration

	
	// New variables for timer/counter indicated packet processing
	volatile int local_captured_ADC_buf_DMA_write = 0;
	volatile U32 local_captured_num_remaining = 0;
	static int prev_captured_ADC_buf_DMA_write = 0;
	static U32 prev_captured_num_remaining = 0;
	U32 last_written_ADC_pos = 0;
	int last_written_ADC_buf = 0;
	static U32 prev_last_written_ADC_pos = 0;
	static int prev_last_written_ADC_buf = 0;


	// Begin new code for timer/counter indicated packet processing

	// Does spdif timer interrupt indicate that we should process 250-ish µs of incoming SPDIF data?
	
	// First establish a local, synchronously-sampled local cache
	local_captured_ADC_buf_DMA_write = timer_captured_ADC_buf_DMA_write;
	local_captured_num_remaining = timer_captured_num_remaining;
	// Interrupt may strike at any time, so re-cache if needed
	if (local_captured_ADC_buf_DMA_write != timer_captured_ADC_buf_DMA_write ) {
		local_captured_ADC_buf_DMA_write = timer_captured_ADC_buf_DMA_write;
		local_captured_num_remaining = timer_captured_num_remaining;
	}
	

	// Reused test based on .reliable from old code below. What is its purpose? It messes up USB playback if it was started during an spdif playback which was later halted
	// NB: For now, spdif_rx_status.reliable = 1 is only set after a mutex take in wm8804.c. Is that correct?

//	if (spdif_rx_status.reliable == 0) { // Temporarily unreliable counts as silent and halts processing
//		spdif_rx_status.silent = 1;
//	}
//	else 

#define NUM_ADDRESSES 10
// U32 addresses[NUM_ADDRESSES];
// int addresses_logger = 0;
	
	if ( (prev_captured_num_remaining != local_captured_num_remaining) || (prev_captured_ADC_buf_DMA_write != local_captured_ADC_buf_DMA_write) ) {

		gpio_set_gpio_pin(AVR32_PIN_PA22); // Indicate start of processing spdif data, ideally once per 250us

		// Start processing a 250µs chunk of the ADC pdca buffer

		// Convert from pdca report to buffer address. _pos always points to left sample in LR stereo pair!
		mobo_ADC_position(&last_written_ADC_pos, &last_written_ADC_buf, local_captured_num_remaining, local_captured_ADC_buf_DMA_write);

		bool we_own_cache = FALSE;			// Cached if-test result
		bool non_silence_det = FALSE;		// We're looking for first non-zero audio-data
		if ( ( (input_select == MOBO_SRC_SPDIF0) || (input_select == MOBO_SRC_TOSLINK0) || (input_select == MOBO_SRC_TOSLINK1) ) && (dac_must_clear == DAC_READY) ) {
			we_own_cache = TRUE;
			si_score_low = 0x7FFFFFFF;		// Highest positive number, reset for each iteration
			(*si_index_low) = 0;			// Location of "lowest energy", reset for each iteration
			(*si_score_high) = 0;			// Lowest positive number, reset for each iteration
			(*si_index_high) = 0;			// Location of "highest energy", reset for each iteration
			(*num_samples) = 0;				// Used to validate cache with non-zero length
		}
		
		int bufpointer = prev_last_written_ADC_buf;	// The first sample to consider for zero detection and data fetch - could possibly reuse prev_last_written_ADC_buf but that would obfuscate readability
		i = prev_last_written_ADC_pos;
//		U32 cachepointer = 0;								

		while (i != last_written_ADC_pos) {
			// Fill endpoint with sample raw
			if (bufpointer == 0) {					// 0 Seems better than 1, but non-conclusive
				sample_L = audio_buffer_0[i];
				sample_R = audio_buffer_0[i + 1];
			}
			else if (bufpointer == 1) {
				sample_L = audio_buffer_1[i];
				sample_R = audio_buffer_1[i + 1];
			}
			
/*			
			if (addresses_logger < NUM_ADDRESSES) {
				addresses[addresses_logger++] = i + (bufpointer << 16);
			}
*/			
				
			i+=2; // counts up to last_written_ADC_buf
			if (i >= ADC_BUFFER_SIZE) {
				i = 0;							// Start from beginning of next buffer
				bufpointer = 1 - bufpointer;	// Toggle buffers
			}
			
			// Silence detect v3.0. Starts out as FALSE, remains TRUE after 1st detection of non-zero audio data 
			non_silence_det = ( non_silence_det || (abs(sample_L) > IS_SILENT) || (abs(sample_R) > IS_SILENT) );

			// It is time consuming to test for each stereo sample!
			if (we_own_cache) {					// Only write to cache and num_samples with the right permissions! And only bother with enerby math if it's considered by calling function
				// Finding packet's point of lowest and highest "energy"
				diff_value = abs( (sample_L >> 8) - (prev_sample_L >> 8) ) + abs( (sample_R >> 8) - (prev_sample_R >> 8) ); // The "energy" going from prev_sample to sample
				diff_sum = diff_value + prev_diff_value; // Add the energy going from prev_prev_sample to prev_sample.
								
				if ((*num_samples) < SPK_CACHE_MAX_SAMPLES) {
					if (diff_sum < si_score_low) {
						si_score_low = diff_sum;
						(*si_index_low) = (*num_samples);
					}
								
					if (diff_sum > (*si_score_high)) {
						(*si_score_high) = diff_sum;
						(*si_index_high) = (*num_samples);
					}
								
					cache_L[(*num_samples)] = prev_sample_L;	// May reuse (*numsamples) 
					cache_R[(*num_samples)] = prev_sample_R;
//					cachepointer++;
					(*num_samples)++;
				} // SPK_CACHE_MAX_SAMPLES
				else {
					print_dbg_char('!'); // Buffer length warning
				}
				
			} // End we_own_cache
								
			// Establish history
			prev_sample_L = sample_L;
			prev_sample_R = sample_R;
			prev_diff_value = diff_value;

		} // while (i != last_written_ADC_pos) 
		
		// Do this once instead of for each sample
//		if (we_own_cache) {
//			*num_samples = cachepointer;
//		}

		
		if (non_silence_det) {
			spdif_rx_status.silent = 0;
		}
		else {
//			print_dbg_char('S');
			spdif_rx_status.silent = 1;
		}
		
/*		
		if (max_last_written_ADC_pos == 0x10101010) {
			
			addresses_logger = 0;
			print_dbg_char('\n');
			while (addresses_logger < NUM_ADDRESSES) {
				print_dbg_hex(addresses[addresses_logger++]);
				print_dbg_char('\n');
			}
			addresses_logger = 0;
			
			max_last_written_ADC_pos = 0;
		}
*/		
		
		
		// Establish history - What to do at player start? Should it be continuously updated at idle? What about spdif source toggle?
		prev_captured_ADC_buf_DMA_write = local_captured_ADC_buf_DMA_write;
		prev_captured_num_remaining = local_captured_num_remaining;
		prev_last_written_ADC_pos = last_written_ADC_pos;
		prev_last_written_ADC_buf = last_written_ADC_buf; 
		
		
		gpio_clr_gpio_pin(AVR32_PIN_PA22); // Indicate end of processing spdif data, ideally once per 250us
		
	} // if ( (prev_captured_num_remaining != local_captured_num_remaining) || (prev_captured_ADC_buf_DMA_write != local_captured_ADC_buf_DMA_write) ) {


	// End new code for timer/counter indicated packet processing



	local_ADC_buf_DMA_write = ADC_buf_DMA_write; // Interrupt may strike at any time, make cached copy for testing below

	// Continue writing to consumer's buffer where this routine left of last
	if ( (prev_ADC_buf_DMA_write == INIT_ADC_I2S) || (ADC_buf_I2S_IN == INIT_ADC_I2S) ) {	// Do the init on synchronous sampling ref. ADC DMA timing

		// Clear incoming SPDIF before enabling pdca to keep filling it - moved to pdca rx enable code
		// mobo_clear_adc_channel();

// Start of new code


		// What is a valid starting point for ADC buffer readout? wm8804 code supposedly just started the pdca for us to be here
		local_captured_ADC_buf_DMA_write = ADC_buf_DMA_write;
		local_captured_num_remaining = pdca_channel->tcr;
		// Interrupt may strike at any time, so re-cache if needed 
		if (local_captured_ADC_buf_DMA_write != ADC_buf_DMA_write ) {
			local_captured_ADC_buf_DMA_write = ADC_buf_DMA_write;
			local_captured_num_remaining = pdca_channel->tcr;
		}

		// Convert from pdca report to buffer address - initiate the present versions of these variables based on the most updated cached readout of ADC pdca status
		// _pos always points to left sample in LR stereo pair!
		mobo_ADC_position(&last_written_ADC_pos, &last_written_ADC_buf, local_captured_num_remaining, local_captured_ADC_buf_DMA_write);
		
		// Clear the history
		prev_captured_ADC_buf_DMA_write = local_captured_ADC_buf_DMA_write;
		prev_captured_num_remaining = local_captured_num_remaining;
		prev_last_written_ADC_pos = last_written_ADC_pos;
		prev_last_written_ADC_buf = last_written_ADC_buf;
		



// End of new code

// Begin old code		

		// Forward state machine
		ADC_buf_I2S_IN = INIT_ADC_I2S_st2;	// Move on to init stage 2

		// Establish history
		prev_ADC_buf_DMA_write = local_ADC_buf_DMA_write;
	} // end INIT_ADC_I2S




// NB: For now, spdif_rx_status.reliable = 1 is only set after a mutex take in wm8804.c. Is that correct?

	if (spdif_rx_status.reliable == 0) { // Temporarily unreliable counts as silent and halts processing
		spdif_rx_status.silent = 1;
		prev_ADC_buf_DMA_write = local_ADC_buf_DMA_write;			// Respond as soon as .reliable is set
	}

	// Has producer's buffer been toggled by interrupt driven DMA code?
	// If so, check it for silence. If selected as source, copy all of producer's data
	// Only bother if .reliable != 0 
	else if (local_ADC_buf_DMA_write != prev_ADC_buf_DMA_write) { // Check if producer has sent more data

		// Establish history
		prev_ADC_buf_DMA_write = local_ADC_buf_DMA_write;

		if ( ( (input_select == MOBO_SRC_TOSLINK0) || (input_select == MOBO_SRC_TOSLINK1) || (input_select == MOBO_SRC_SPDIF0) ) ) {

			// Startup condition: must initiate consumer's write pointer to where-ever its read pointer may be
			// æææææ does this influence uac2_dat and where cache data is written?
			if (ADC_buf_I2S_IN == INIT_ADC_I2S_st2) {

				ADC_buf_I2S_IN = local_ADC_buf_DMA_write;	// Disable further init, select correct audio_buffer_0/1
				dac_must_clear = DAC_READY;					// Prepare to send actual data to DAC interface

				// New co-sample verification routine
				local_DAC_buf_DMA_read = DAC_buf_DMA_read;
				num_remaining = spk_pdca_channel->tcr;
				// Did an interrupt strike just ? Check if DAC_buf_DMA_read is valid. If not valid, interrupt won't strike again for a long time. In which we simply read the counter again
				if (local_DAC_buf_DMA_read != DAC_buf_DMA_read) {
					local_DAC_buf_DMA_read = DAC_buf_DMA_read;
					num_remaining = spk_pdca_channel->tcr;
				}
				DAC_buf_OUT = local_DAC_buf_DMA_read;

				// Where to start writing to spk_index? Is that relevant when writing through cache?
				spk_index = DAC_BUFFER_SIZE - num_remaining;
				spk_index = spk_index & ~((U32)1); 	// Clear LSB in order to start with L sample
			} // if (ADC_buf_I2S_IN == INIT_ADC_I2S_st2)

			// Prepare to copy all of producer's most recent data to consumer's buffer
			if (local_ADC_buf_DMA_write == 1)
				gpio_set_gpio_pin(AVR32_PIN_PX18);			// Pin 84
			else if (local_ADC_buf_DMA_write == 0)
				gpio_clr_gpio_pin(AVR32_PIN_PX18);			// Pin 84


//			gpio_set_gpio_pin(AVR32_PIN_PX30);		// Indicate copying DAC data from audio_buffer_X to spk_audio_buffer_X

			// Now: 
			// - Processing a whole ADC_BUFFER_SIZE audio_buffer_0 or _1
			// - s/i removed
			// - Gap calculation completely removed
			
			// Next:
			// - Processing one packet at a time
			// - Replicate "energy" code and sample delay from uac2_dat where it writes to cache
			// + Zero detection here
			// - New gap calculation, uac2_dat
			// - Write through cache, init in uac2_dat
			// - Detect need for s/i here, execute it with reads from cache

/*
			
			for (i=0 ; i < ADC_BUFFER_SIZE ; i+=2) {
				// Fill endpoint with sample raw
				if (local_ADC_buf_DMA_write == 0) {		// 0 Seems better than 1, but non-conclusive
					sample_L = audio_buffer_0[i];
					sample_R = audio_buffer_0[i + 1];
				}
				else if (local_ADC_buf_DMA_write == 1) {
					sample_L = audio_buffer_1[i];
					sample_R = audio_buffer_1[i + 1];
				}

				if (dac_must_clear == DAC_READY) {
					if (DAC_buf_OUT == 0) {
//						spk_buffer_0[spk_index] = sample_L;
//						spk_buffer_0[spk_index + 1] = sample_R;
					}
					else if (DAC_buf_OUT == 1) {
//						spk_buffer_1[spk_index] = sample_L;
//						spk_buffer_1[spk_index + 1] = sample_R;
					}
				}

				spk_index += 2;
				if (spk_index >= DAC_BUFFER_SIZE) {
					spk_index -= DAC_BUFFER_SIZE;
					DAC_buf_OUT = 1 - DAC_buf_OUT;

#ifdef USB_STATE_MACHINE_DEBUG
//							if (DAC_buf_OUT == 1)
//								gpio_set_gpio_pin(AVR32_PIN_PX30);
//							else
//								gpio_clr_gpio_pin(AVR32_PIN_PX30);
#endif
				} // if (spk_index >= DAC_BUFFER_SIZE)

			} // for ADC_BUFFER_SIZE 
			
*/			
				
//			gpio_clr_gpio_pin(AVR32_PIN_PX30);		// Indicate copying DAC data from audio_buffer_X to spk_audio_buffer_X
				

		} // input select
		
	} // ADC_buf_DMA_write toggle
	




	

} // mobo_handle_spdif(void)



// Start the timer/counter that monitors spdif traffic
void mobo_start_spdif_tc(U32 frequency) {
	volatile avr32_tc_t *tc = &SPDIF_TC_DEVICE;
	uint8_t temp = 0;

	switch (frequency) {
		case FREQ_44:
			temp = 11;	// UAC2: 11.025 samples per 250µs
		break;
		case FREQ_48:
			temp = 12;	// UAC2: 12 samples per 250µs
		break;
		case FREQ_88:
			temp = 22;	// UAC2: 22.05 samples per 250µs
		break;
		case FREQ_96:
			temp = 24;	// UAC2: 24 samples per 250µs
		break;
		case FREQ_176:
			temp = 44;	// UAC2: 44.1 samples per 250µs
		break;
		case FREQ_192:
			temp = 48;	// UAC2: 48 samples per 250µs
		break;
		case FREQ_RXNATIVE:
			temp = 12;	// UAC2: 11 samples per 250µs - risky, we may have to process packets more often than every 250µs
		break;
		default:
			temp = 12;	// UAC2: 11 samples per 250µs
		break;
	}			
	
	// Configure the timer limit register
	tc_write_rc(tc, SPDIF_TC_CHANNEL, temp);

	// Start the timer/counter, includes resetting the timer value to 0!
	tc_start(tc, SPDIF_TC_CHANNEL); // Implements SWTRG software trig and CLKEN clock enable
}


// Stop the timer/counter that monitors spdif traffic
void mobo_stop_spdif_tc(void) {
	volatile avr32_tc_t *tc = &SPDIF_TC_DEVICE;

	tc_stop(tc, SPDIF_TC_CHANNEL);
}



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

		// New version, without I2S mux, with buffering via MCU's ADC interface
		// RXmod_t1_B has the MUX built-in but sets SEL_USBP_RXN = PC01 to always select MCU's outgoing I2S port toward the DAC chip
		gpio_set_gpio_pin(AVR32_PIN_PC01); 			// SEL_USBP_RXN = 1 defaults to USB and buffering via MCU FIFO - not used on RXMOD C and onwards

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
		// FREQ_INVALID defaults to 48kHz domain? Is that consistent in code?
		else {
			gpio_set_gpio_pin(AVR32_PIN_PA21); 		// 48 control
			gpio_clr_gpio_pin(AVR32_PIN_PA23); 		// 44.1 control
			gpio_clr_gpio_pin(AVR32_PIN_PX22); 		// Disable RX recovered MCLK
		}

		// Report to CPU (when present) and debug terminal
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

	#elif (defined HW_GEN_FMADC)
		// FMADC_site 
		// 96ksps domain is permanently turned on
	#else
		#error undefined hardware
	#endif

		// Establish history
		prev_frequency = frequency;
	} // if (frequency != prev_frequency)
}


// Master clock to DAC's I2S port frequency setup
void mobo_clock_division(U32 frequency) {

	static U32 prev_frequency = FREQ_INVALID;

	if ( (frequency != prev_frequency) || (prev_frequency == FREQ_INVALID) ) { 	// Only run at startup or when things change
		
/* 20230930: Clock division option re-introduced in HW_GEN_RXMOD. NBNBNB Detect pin polarity is the opposite from that of AB-1.2 series. FMADC has no division!*/
		
		gpio_enable_pin_pull_up(AVR32_PIN_PA03);	// Floating: stock AW with external /2. GND: modded AW with no ext. /2
	
		pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

		#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC)
			// RXMOD: Pulling down PA03 indicates division. Floating indicates NO division.
			if (gpio_get_pin_value(AVR32_PIN_PA03) == 0) {
		#else
			// AB-1.2, USB DAC 128: Pulling down PA03 indicates NO division. Floating indicates division.
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

		// No external /2 variety
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
				default :		// Treated as 44.1
					pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								1,                  // diven - enabled
								3);                 // divided by 8
				break;
			}
		}

		gpio_disable_pin_pull_up(AVR32_PIN_PA03);	// Free up the pin again

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




