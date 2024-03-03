/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */

/* Written by Borge Strand-Bergesen 20211116
 * Interaction with PCM5142
 *
 * Copyright (C) Borge Strand-Bergesen, borge@henryaudio.com
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



#if (defined HW_GEN_SPRX)		// Functions here only make sense for WM8804

#include "pcm5142.h"
#include "gpio.h"
// #include "Mobo_config.h"
#include "features.h"
#include "device_audio_task.h"
#include "usb_specific_request.h"
#include "Mobo_config.h"
#include "I2C.h"
#include "pdca.h" // To disable DMA at sleep
#include "taskAK5394A.h" // To signal uacX_device_audio_task to enable DMA at init



// Select input built-in interpolation filter
void pcm5142_filter(uint8_t filter_sel) {
//	print_dbg_char(0x30 + filter_sel);

	// For valid filters see datasheet section 8.3.4.2 / Register 43 (dec) / 0x2B
	if ( (filter_sel == 0x01) || (filter_sel == 0x02) || (filter_sel == 0x03) || (filter_sel == 0x07) ) {
		pcm5142_write_byte(0x00, 0x00);			// Page 0
		pcm5142_write_byte(0x02, 0x10);			// P00 A02 Standby request
		pcm5142_write_byte(0x2B, filter_sel);	// P00 A2b Actual filter
		pcm5142_write_byte(0x02, 0x00);			// P00 A02 Exit standby
		print_dbg_char('F');					// Verify selection
	}
}


// Write a single byte to PCM5142
uint8_t pcm5142_write_byte(uint8_t int_adr, uint8_t int_data) {
    uint8_t dev_data[2];
    uint8_t status = 0xFF;							// Far from 0 reported as I2C success

	// Wrap entire I2C transfer in semaphore, not just each I2C/twi function call
//	print_dbg_char('a'); 
	if (xSemaphoreTake(I2C_busy_semphr, 0) == pdTRUE) {	// Re-take of taken semaphore returns false
//		print_dbg_char('A');


		// How long delay is needed? Should we bring this out of semaphore code? That might interfere with ongoing transfers, but not take up time
		gpio_set_gpio_pin(AVR32_PIN_PX17);		// I2C enable for DAC in rev. C, unused control pin in rev. A and B
		vTaskDelay(5);							// Wait 0.5ms

		// Start of blocking code
		#ifdef HW_GEN_SPRX_PATCH_02
			gpio_set_gpio_pin(AVR32_PIN_PX17);		// M_DAC_I2C_EN enable I2C to DAC
			vTaskDelay(10);							// Wild guess at delay time
		#endif

		dev_data[0] = int_adr;
		dev_data[1] = int_data;
		status = twi_write_out(PCM5142_DEV_ADR, dev_data, 2);

		#ifdef HW_GEN_SPRX_PATCH_02
			gpio_clr_gpio_pin(AVR32_PIN_PX17);		// M_DAC_I2C_EN, cut off I2C noise to DAC
		#endif
		// End of blocking code

		gpio_clr_gpio_pin(AVR32_PIN_PX17);		// I2C disable for DAC
		vTaskDelay(5);							// Wait 0.5ms

//		print_dbg_char('g');
		if( xSemaphoreGive(I2C_busy_semphr) == pdTRUE ) {
//			print_dbg_char(60); // '<'
		}
		else {
			print_dbg_char('P');
		}
	}
	else {
		print_dbg_char('Q');
	}


	return status;
}


// Read a single byte from PCM5142
uint8_t pcm5142_read_byte(uint8_t int_adr) {
	uint8_t dev_data[1];
	
	// Wrap entire I2C transfer in semaphore, not just each I2C/twi function call
//	print_dbg_char('b');
	if (xSemaphoreTake(I2C_busy_semphr, 0) == pdTRUE) {	// Re-take of taken semaphore returns false
//		print_dbg_char('B');

		// How long delay is needed? Should we bring this out of semaphore code? That might interfere with ongoing transfers, but not take up time
		gpio_set_gpio_pin(AVR32_PIN_PX17);		// I2C enable for DAC in rev. C, unused control pin in rev. A and B
		vTaskDelay(5);							// Wait 0.5ms

		// Start of blocking code
		#ifdef HW_GEN_SPRX_PATCH_02
			gpio_set_gpio_pin(AVR32_PIN_PX17);		// M_DAC_I2C_EN enable I2C to DAC
			vTaskDelay(10);							// Wild guess at delay time
		#endif

		dev_data[0] = int_adr;
		if (twi_write_out(PCM5142_DEV_ADR, dev_data, 1) == TWI_SUCCESS) {
			twi_read_in(PCM5142_DEV_ADR, dev_data, 1);
		}
		else
			dev_data[0] = 0 ;	// Randomly chosen failure state

		#ifdef HW_GEN_SPRX_PATCH_02
			gpio_clr_gpio_pin(AVR32_PIN_PX17);		// M_DAC_I2C_EN, cut off I2C noise to DAC
		#endif
		// End of blocking code

		gpio_clr_gpio_pin(AVR32_PIN_PX17);		// I2C disable for DAC
		vTaskDelay(5);							// Wait 0.5ms

//		print_dbg_char('g');
		if( xSemaphoreGive(I2C_busy_semphr) == pdTRUE ) {
//			print_dbg_char(60); // '<'
		}
		else {
			print_dbg_char('Q');
		}
	}
	else {
		print_dbg_char('T');
	}

	return dev_data[0];
}


#endif  // HW_GEN_SPRX
