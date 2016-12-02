/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * hpsdr_taskAK5394A.c
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
#include "Mobo_config.h"
#include "pdca.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "device_audio_task.h"
#include "hpsdr_device_audio_task.h"
#include "taskAK5394A.h"
#include "uac2_taskAK5394A.h"

//_____ M A C R O S ________________________________________________________

//_____ D E F I N I T I O N S ______________________________________________

//_____ D E C L A R A T I O N S ____________________________________________

void hpsdr_AK5394A_task(void*);

//!
//! @brief This function initializes the hardware/software resources
//! required for device CDC task.
//!
void hpsdr_AK5394A_task_init(void) {
	current_freq.frequency = FREQ_48;
	AK5394A_task_init(TRUE);
	xTaskCreate(hpsdr_AK5394A_task,
				configTSK_AK5394A_NAME,
				configTSK_AK5394A_STACK_SIZE,
				NULL,
				HPSDR_configTSK_AK5394A_PRIORITY,
				NULL);
}

//!
//! @brief Entry point of the AK5394A task management
//!
void hpsdr_AK5394A_task(void *pvParameters) {
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	while (TRUE) {
		// All the hardwork is done by the pdca and the interrupt handler.
		// Just check whether sampling freq is changed, to do rate change etc.

		vTaskDelayUntil(&xLastWakeTime, HPSDR_configTSK_AK5394A_PERIOD);

		if (freq_changed) {

			if (current_freq.frequency == FREQ_96) {
				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_disable(PDCA_CHANNEL_SSC_RX);

				gpio_set_gpio_pin(AK5394_DFS0);		// L H  -> 96khz
				gpio_clr_gpio_pin(AK5394_DFS1);

				mobo_clock_division(current_freq.frequency);

				FB_rate = 96 << 14;
    			FB_rate_initial = FB_rate;							// BSB 20131031 Record FB_rate as it was set by control system
    			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET;		// BSB 20131115 Record FB_rate as it was set by control system;

			} else if (current_freq.frequency == FREQ_192) {
				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_disable(PDCA_CHANNEL_SSC_RX);

				gpio_clr_gpio_pin(AK5394_DFS0);		// H L -> 192khz
				gpio_set_gpio_pin(AK5394_DFS1);

				mobo_clock_division(current_freq.frequency);

				FB_rate = 192 << 14;
    			FB_rate_initial = FB_rate;							// BSB 20131031 Record FB_rate as it was set by control system
    			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET;		// BSB 20131115 Record FB_rate as it was set by control system;

			} else if (current_freq.frequency == FREQ_48) {

				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_disable(PDCA_CHANNEL_SSC_RX);

				gpio_clr_gpio_pin(AK5394_DFS0);		// L H  -> 96khz L L  -> 48khz
				gpio_clr_gpio_pin(AK5394_DFS1);

				mobo_clock_division(current_freq.frequency);

				FB_rate = 48 << 14;
    			FB_rate_initial = FB_rate;							// BSB 20131031 Record FB_rate as it was set by control system
    			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET;		// BSB 20131115 Record FB_rate as it was set by control system;

			}

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

			// reset freq_changed flag
			freq_changed = FALSE;
		}

	} // end while (TRUE)
}


