/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * uac2_taskAK5394A.c
 *
 *  Created on: Feb 14, 2010
 *  Refactored on: Feb 26, 2011
 *      Author: Alex
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
 *
 */

//_____  I N C L U D E S ___________________________________________________

//#include <stdio.h>
#include "usart.h"     // Shall be included before FreeRTOS header files, since 'inline' is defined to ''; leading to
                       // link errors
#include "conf_usb.h"


#include <avr32/io.h>
#if __GNUC__
#  include "intc.h"
#endif
#include "board.h"
#ifdef FREERTOS_USED
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "usb_drv.h"
#include "gpio.h"
#include "ssc_i2s.h"
#include "pm.h"
#include "pdca.h"
#include "usb_standard_request.h"
#include "features.h"
#include "usb_specific_request.h"
#include "device_audio_task.h"
#include "uac2_device_audio_task.h"
#include "taskAK5394A.h"
#include "uac2_taskAK5394A.h"
#include "Mobo_config.h"

//_____ M A C R O S ________________________________________________________

//_____ D E F I N I T I O N S ______________________________________________

//_____ D E C L A R A T I O N S ____________________________________________

void uac2_AK5394A_task(void*);

//!
//! @brief This function initializes the hardware/software resources
//! required for device CDC task.
//!
void uac2_AK5394A_task_init(void) {
	current_freq.frequency = 96000;
	AK5394A_task_init(FALSE);
	xTaskCreate(uac2_AK5394A_task,
				configTSK_AK5394A_NAME,
				configTSK_AK5394A_STACK_SIZE,
				NULL,
				UAC2_configTSK_AK5394A_PRIORITY,
				NULL);
}

//!
//! @brief Entry point of the AK5394A task management
//!
void uac2_AK5394A_task(void *pvParameters) {
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	int i;

	while (TRUE) {
		// All the hardwork is done by the pdca and the interrupt handler.
		// Just check whether sampling freq is changed, to do rate change etc.

		vTaskDelayUntil(&xLastWakeTime, UAC2_configTSK_AK5394A_PERIOD);

		if (freq_changed) {

			if (current_freq.frequency == 96000) {
				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_disable(PDCA_CHANNEL_SSC_RX);

				if (FEATURE_BOARD_USBI2S)
					gpio_set_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 24.576MHz/2 for AB-1
				else if (FEATURE_BOARD_USBDAC)
					gpio_set_gpio_pin(AVR32_PIN_PX51);

				if ( FEATURE_ADC_AK5394A ) {
					gpio_set_gpio_pin(AK5394_DFS0);		// L H  -> 96khz
					gpio_clr_gpio_pin(AK5394_DFS1);
				}

				pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);
				pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
							0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
							1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
							1,                  // diven - enabled
							0);                 // divided by 2.  Therefore GCLK1 = 6.144Mhz
				pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

				FB_rate = 96 << 14;

			}

		   	else if (current_freq.frequency == 88200){
				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_disable(PDCA_CHANNEL_SSC_RX);

				if (FEATURE_BOARD_USBI2S)
					gpio_clr_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 22.5792MHz/2 for AB-1
				else if (FEATURE_BOARD_USBDAC)
					gpio_clr_gpio_pin(AVR32_PIN_PX51);

				if ( FEATURE_ADC_AK5394A ) {
					gpio_set_gpio_pin(AK5394_DFS0);		// L H  -> 96khz
					gpio_clr_gpio_pin(AK5394_DFS1);
				}

				pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);
				pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
								  0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
								  1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
								  1,                  // diven - enabled
								  0);                 // divided by 2.  Therefore GCLK1 = 6.144Mhz
				pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

				FB_rate = (88 << 14) + (1<<14)/5;
				}

	       	else if (current_freq.frequency == 176400)
	        	{
	    			pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
	    			pdca_disable(PDCA_CHANNEL_SSC_RX);

					if (FEATURE_BOARD_USBI2S)
						gpio_clr_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 22.5792MHz/2 for AB-1
					else if (FEATURE_BOARD_USBDAC)
						gpio_clr_gpio_pin(AVR32_PIN_PX51);

	    			gpio_clr_gpio_pin(AK5394_DFS0);		// H L -> 192khz
	    			gpio_set_gpio_pin(AK5394_DFS1);

	    			pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);
	    			pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
	    								  0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
	    								  1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
	    								  0,                  // diven - disabled
	    								  0);                 // GCLK1 = 12.288Mhz
	    			pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

	    			FB_rate = (176 << 14) + ((1<<14) * 4)/ 10;

	        	}

			else if (current_freq.frequency == 192000) {
				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_disable(PDCA_CHANNEL_SSC_RX);

				if (FEATURE_BOARD_USBI2S)
					gpio_set_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 24.576MHz/2 for AB-1
				else if (FEATURE_BOARD_USBDAC)
					gpio_set_gpio_pin(AVR32_PIN_PX51);

				if ( FEATURE_ADC_AK5394A ) {
					gpio_clr_gpio_pin(AK5394_DFS0);		// H L -> 192khz
					gpio_set_gpio_pin(AK5394_DFS1);
				}

				pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);
				pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
							0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
							1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
							0,                  // diven - disabled
							0);                 // GCLK1 = 12.288Mhz
				pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

				FB_rate = 192 << 14;

			} else if (current_freq.frequency == 48000) {
				// if there are two XO, PX16 sets the 48x
				// gpio_set_gpio_pin(AVR32_PIN_PX16);
				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_disable(PDCA_CHANNEL_SSC_RX);

				if (FEATURE_BOARD_USBI2S)
					gpio_set_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 24.576MHz/2 for AB-1
				else if (FEATURE_BOARD_USBDAC)
					gpio_set_gpio_pin(AVR32_PIN_PX51);

				if ( FEATURE_ADC_AK5394A ) {
					gpio_clr_gpio_pin(AK5394_DFS0);		// L H  -> 96khz L L  -> 48khz
					gpio_clr_gpio_pin(AK5394_DFS1);
				}

				pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);
				pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
							0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
							1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
							1,                  // diven - enabled
							1);                 // divided by 4.  Therefore GCLK1 = 3.072Mhz
				pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

				FB_rate = 48 << 14;

			}

			else if (current_freq.frequency == 44100) {
				// if there are two XO, PX16 set --> 48x. clr -->44.1x
				// gpio_clr_gpio_pin(AVR32_PIN_PX16);
				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_disable(PDCA_CHANNEL_SSC_RX);

				if (FEATURE_BOARD_USBI2S)
					gpio_clr_gpio_pin(AVR32_PIN_PX16); // BSB 20110301 MUX in 22.5792MHz/2 for AB-1
				else if (FEATURE_BOARD_USBDAC)
					gpio_clr_gpio_pin(AVR32_PIN_PX51);

				if ( FEATURE_ADC_AK5394A ) {
					gpio_clr_gpio_pin(AK5394_DFS0);		// L H  -> 96khz L L  -> 48khz
					gpio_clr_gpio_pin(AK5394_DFS1);
				}

				pm_gc_disable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);
				pm_gc_setup(&AVR32_PM, AVR32_PM_GCLK_GCLK1, // gc
							0,                  // osc_or_pll: use Osc (if 0) or PLL (if 1)
							1,                  // pll_osc: select Osc0/PLL0 or Osc1/PLL1
							1,                  // diven - enabled
							1);                 // divided by 4.  Therefore GCLK1 = 3.072Mhz
				pm_gc_enable(&AVR32_PM, AVR32_PM_GCLK_GCLK1);

				FB_rate = (44 << 14) + (1 << 14)/10 ;

			}

			if (FEATURE_ADC_AK5394A) {
				// re-sync SSC to LRCK
				// Wait for the next frame synchronization event
				// to avoid channel inversion.  Start with left channel - FS goes low
				// However, the channels are reversed at 192khz

				if (current_freq.frequency == 192000) {
					while (gpio_get_pin_value(AK5394_LRCK));
					while (!gpio_get_pin_value(AK5394_LRCK));	// exit when FS goes high
				} else {
					while (!gpio_get_pin_value(AK5394_LRCK));
					while (gpio_get_pin_value(AK5394_LRCK));	// exit when FS goes low
				}
				// Enable now the transfer.
				pdca_enable(PDCA_CHANNEL_SSC_RX);

				// Init PDCA channel with the pdca_options.
				AK5394A_pdca_enable();
			}

			// reset freq_changed flag
			freq_changed = FALSE;
		}

		if (usb_alternate_setting_out_changed) {
			if (usb_alternate_setting_out != 1) {
				for (i = 0; i < SPK_BUFFER_SIZE; i++) {
					spk_buffer_0[i] = 0;
					spk_buffer_1[i] = 0;
				}
			}
			usb_alternate_setting_out_changed = FALSE;
		}

		if (FEATURE_IMAGE_UAC2_DG8SAQ) {
			spk_mute = TX_state ? FALSE : TRUE;
			mute = TX_state ? TRUE : FALSE;
		}


	} // end while (TRUE)
}


