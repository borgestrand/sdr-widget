/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * uac1_taskAK5394A.c
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
#include "features.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "device_audio_task.h"
#include "uac1_device_audio_task.h"
#include "taskAK5394A.h"
#include "uac1_taskAK5394A.h"
#include "Mobo_config.h"

//_____ M A C R O S ________________________________________________________

//_____ D E F I N I T I O N S ______________________________________________

//_____ D E C L A R A T I O N S ____________________________________________

void uac1_AK5394A_task(void*);

//!
//! @brief This function initializes the hardware/software resources
//! required for device CDC task.
//!
void uac1_AK5394A_task_init(void) {
	AK5394A_task_init(TRUE);
	xTaskCreate(uac1_AK5394A_task,
				configTSK_AK5394A_NAME,
				configTSK_AK5394A_STACK_SIZE,
				NULL,
				UAC1_configTSK_AK5394A_PRIORITY,
				NULL);
}

//!
//! @brief Entry point of the AK5394A task management
//!
void uac1_AK5394A_task(void *pvParameters) {
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	int i;
	volatile S32 usb_buffer_toggle;

	while (TRUE) {
		// All the hardwork is done by the pdca and the interrupt handler.
		// Just check whether alternate setting is changed, to do rate change etc.

		vTaskDelayUntil(&xLastWakeTime, UAC1_configTSK_AK5394A_PERIOD);

		if (freq_changed) {
			spk_mute = TRUE;
			if (current_freq.frequency == FREQ_48) {
				FB_rate = 48 << 14;
    			FB_rate_initial = FB_rate;							// BSB 20131031 Record FB_rate as it was set by control system
    			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET;		// BSB 20131115 Record FB_rate as it was set by control system;
}
			else {
				FB_rate = (44 << 14) + (1 << 14)/10 ;
    			FB_rate_initial = FB_rate;							// BSB 20131031 Record FB_rate as it was set by control system
    			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET;		// BSB 20131115 Record FB_rate as it was set by control system;
}
			spk_mute = FALSE;
			freq_changed = FALSE;
		}
		if (usb_alternate_setting_changed) {

			pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
			pdca_disable(PDCA_CHANNEL_SSC_RX);
			// L L  -> 48khz   L H  -> 96khz
			gpio_clr_gpio_pin(AK5394_DFS0);
			gpio_clr_gpio_pin(AK5394_DFS1);

			if (FEATURE_ADC_AK5394A) {
				// re-sync SSC to LRCK
				// Wait for the next frame synchronization event
				// to avoid channel inversion.  Start with left channel - FS goes low
				while (!gpio_get_pin_value(AK5394_LRCK));
				while (gpio_get_pin_value(AK5394_LRCK));

				// Enable now the transfer.
				pdca_enable(PDCA_CHANNEL_SSC_RX);

				// Init PDCA channel with the pdca_options.
				AK5394A_pdca_enable();
			}
			// reset usb_alternate_setting_changed flag
			usb_alternate_setting_changed = FALSE;
		}

/*
		if (usb_alternate_setting_out_changed){
			if (usb_alternate_setting_out != 1){
				spk_mute = TRUE;
				for (i = 0; i < DAC_BUFFER_SIZE; i++){
					spk_buffer_0[i] = 0;
					spk_buffer_1[i] = 0;
				}
				spk_mute = FALSE;
			}
			usb_alternate_setting_out_changed = FALSE;
		}
*/

// silence speaker if USB data out is stalled, as indicated by heart-beat counter
		if (old_spk_usb_heart_beat == spk_usb_heart_beat){
			if (input_select == MOBO_SRC_UAC1) {	// balle BSB 20161010 speculative fix for ADC re-use
				for (i = 0; i < DAC_BUFFER_SIZE; i++) {
					spk_buffer_0[i] = 0;
					spk_buffer_1[i] = 0;
				}
			}

			// BSB 20131209 attempting improved playerstarted detection
			// Next iteration of uacX_device_audio_task will set playerStarted to FALSE
			if (usb_buffer_toggle < USB_BUFFER_TOGGLE_LIM)
				usb_buffer_toggle = USB_BUFFER_TOGGLE_LIM;
		}
		old_spk_usb_heart_beat = spk_usb_heart_beat;
/*
		if (FEATURE_IMAGE_UAC1_DG8SAQ) {
			spk_mute = TX_state ? FALSE : TRUE;
			mute = TX_state ? TRUE : FALSE;
		}
*/
	} // end while (TRUE)
}


