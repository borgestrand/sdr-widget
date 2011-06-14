/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Processing of USB device specific enumeration requests.
 *
 * This file contains the specific request decoding for enumeration process.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with a USB module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ***************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 * Additions and Modifications to ATMEL AVR32-SoftwareFramework-AT32UC3 are:
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
 * Modified by Alex Lee and sdr-widget team since Feb 2010.  Copyright General Purpose Licence v2.
 * Please refer to http://code.google.com/p/sdr-widget/
 */

//_____ I N C L U D E S ____________________________________________________

#include "conf_usb.h"


#if USB_DEVICE_FEATURE == ENABLED

#include "usb_drv.h"
#include "usb_descriptors.h"
#include "uac2_usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "usart.h"
#include "pm.h"
#include "features.h"
#include "Mobo_config.h"
#include "usb_audio.h"
#include "device_audio_task.h"
#include "uac2_device_audio_task.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________


//_____ P R I V A T E   D E C L A R A T I O N S ____________________________

// U8 usb_feature_report[3];
// U8 usb_report[3];

// U8 g_u8_report_rate=0;

// S_line_coding   line_coding;

// S_freq current_freq;
// Bool freq_changed = FALSE;

static U8    wValue_msb;
static U8    wValue_lsb;
static U16   wIndex;
static U16   wLength;

extern const    void *pbuffer;
extern          U16   data_to_transfer;

U8 Speedx_1[26] = {
0x02,0x00,				//number of sample rate triplets

0x44,0xac,0x00,0x00,	//44.1k Min
0x10,0xb1,0x02,0x00,	//176.4k Max
0x44,0xac,0x00,0x00,	//44.1k Res

0x80,0xbb,0x00,0x00,	//48k Min
0x00,0xee,0x02,0x00,	//192k Max
0x80,0xbb,0x00,0x00,	//48k Res
};

U8 Speedx_2[38] = {
0x03,0x00,				//number of sample rate triplets

0x80,0xbb,0x00,0x00,	//48k Min
0x80,0xbb,0x00,0x00,	//48k Max
0x00,0x00,0x00,0x00,	// 0 Res

0x00,0x77,0x01,0x00,	//96k Min
0x00,0x77,0x01,0x00,	//96k Max
0x00,0x00,0x00,0x00,	// 0 Res

0x00,0xee,0x02,0x00,	//192k Min
0x00,0xee,0x02,0x00,	//192k Max
0x00,0x00,0x00,0x00		// 0 Res
};

//_____ D E C L A R A T I O N S ____________________________________________


//! @brief This function configures the endpoints of the device application.
//! This function is called when the set configuration request has been received.
//!
void uac2_user_endpoint_init(U8 conf_nb)
{
	if( Is_usb_full_speed_mode() ) {
		(void)Usb_configure_endpoint(UAC2_EP_AUDIO_IN, EP_ATTRIBUTES_1, DIRECTION_IN, EP_SIZE_1_FS, DOUBLE_BANK, 0);
		(void)Usb_configure_endpoint(UAC2_EP_AUDIO_OUT, EP_ATTRIBUTES_2, DIRECTION_OUT, EP_SIZE_2_FS, DOUBLE_BANK, 0);
		(void)Usb_configure_endpoint(UAC2_EP_AUDIO_OUT_FB, EP_ATTRIBUTES_3, DIRECTION_IN, EP_SIZE_3_FS, DOUBLE_BANK, 0);
	} else {
		(void)Usb_configure_endpoint(UAC2_EP_AUDIO_IN, EP_ATTRIBUTES_1, DIRECTION_IN, EP_SIZE_1_HS, DOUBLE_BANK, 0);
		(void)Usb_configure_endpoint(UAC2_EP_AUDIO_OUT, EP_ATTRIBUTES_2, DIRECTION_OUT, EP_SIZE_2_HS, DOUBLE_BANK, 0);
		(void)Usb_configure_endpoint(UAC2_EP_AUDIO_OUT_FB, EP_ATTRIBUTES_3, DIRECTION_IN, EP_SIZE_3_HS, DOUBLE_BANK, 0);
	}
}

//! @brief This function handles usb_set_interface side effects.
//! This function is called from usb_set_interface in usb_standard_request
//! in case there are side effects of the interface change to be handled.
void uac2_user_set_interface(U8 wIndex, U8 wValue) {
   //* Check whether it is the audio streaming interface and Alternate Setting that is being set
   usb_interface_nb = wIndex;
   if (usb_interface_nb == STD_AS_INTERFACE_IN) {
	   usb_alternate_setting = wValue;
	   usb_alternate_setting_changed = TRUE;
   } else if (usb_interface_nb == STD_AS_INTERFACE_OUT){
	   usb_alternate_setting_out = wValue;
	   usb_alternate_setting_out_changed = TRUE;
   }

}

//! This function is called by the standard USB read request function when
//! the USB request is not supported. This function returns TRUE when the
//! request is processed. This function returns FALSE if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard USB read request function.
//!
Bool uac2_user_read_request(U8 type, U8 request)
{   int i;

	// Read wValue
	// why are these file statics?
	wValue_lsb = Usb_read_endpoint_data(EP_CONTROL, 8);
	wValue_msb = Usb_read_endpoint_data(EP_CONTROL, 8);
	wIndex = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
	wLength = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));

	if (type == IN_CL_INTERFACE || type == OUT_CL_INTERFACE){    // process Class Specific Interface


		//  request for AUDIO interfaces

		if (wIndex == DSC_INTERFACE_AS){				// Audio Streaming Interface
			if (type == IN_CL_INTERFACE){			// get controls

				if (wValue_msb == AUDIO_AS_VAL_ALT_SETTINGS && wValue_lsb == 0
					&& request == AUDIO_CS_REQUEST_CUR){
					Usb_ack_setup_received_free();

					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x01);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0b00000011); // alt 0 and 1 valid
					Usb_ack_control_in_ready_send();

					while (!Is_usb_control_out_received());
					Usb_ack_control_out_received_free();
					return TRUE;
				} else if (wValue_msb == AUDIO_AS_ACT_ALT_SETTINGS && wValue_lsb == 0
 						   && request == AUDIO_CS_REQUEST_CUR){
					Usb_ack_setup_received_free();
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, usb_alternate_setting);
					Usb_ack_control_in_ready_send();
					while (!Is_usb_control_out_received());
					Usb_ack_control_out_received_free();
					return TRUE;
				} else if (wValue_msb == AUDIO_AS_AUDIO_DATA_FORMAT && wValue_lsb == 0
 						   && request == AUDIO_CS_REQUEST_CUR){
					Usb_ack_setup_received_free();
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x01);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);	// only PCM format
					Usb_ack_control_in_ready_send();
					while (!Is_usb_control_out_received());
					Usb_ack_control_out_received_free();
					return TRUE;
				} else return FALSE;

			} else if (type == OUT_CL_INTERFACE){		// set controls
				if (wValue_msb == AUDIO_AS_ACT_ALT_SETTINGS
					&& request == AUDIO_CS_REQUEST_CUR){
					Usb_ack_setup_received_free();
					while (!Is_usb_control_out_received());
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					usb_alternate_setting = Usb_read_endpoint_data(EP_CONTROL, 8);
					usb_alternate_setting_changed = TRUE;
					Usb_ack_control_out_received_free();
					Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
					while (!Is_usb_control_in_ready()); //!< waits for status phase done
					return FALSE;
				}
			} // end OUT_CL_INTERFACE
		} // end DSC_INTERFACE_AS

		if (wIndex == DSC_INTERFACE_AS_OUT){				// Playback Audio Streaming Interface
			if (type == IN_CL_INTERFACE){			// get controls

				if (wValue_msb == AUDIO_AS_VAL_ALT_SETTINGS && wValue_lsb == 0
					&& request == AUDIO_CS_REQUEST_CUR){
					Usb_ack_setup_received_free();

					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x01);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0b00000011); // alt 0 and 1 valid
					Usb_ack_control_in_ready_send();

					while (!Is_usb_control_out_received());
					Usb_ack_control_out_received_free();
					return TRUE;
				} else if (wValue_msb == AUDIO_AS_ACT_ALT_SETTINGS && wValue_lsb == 0
 						   && request == AUDIO_CS_REQUEST_CUR){
					Usb_ack_setup_received_free();
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, usb_alternate_setting_out);
					Usb_ack_control_in_ready_send();
					while (!Is_usb_control_out_received());
					Usb_ack_control_out_received_free();
					return TRUE;
				} else if (wValue_msb == AUDIO_AS_AUDIO_DATA_FORMAT && wValue_lsb == 0
 						   && request == AUDIO_CS_REQUEST_CUR){
					Usb_ack_setup_received_free();
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x01);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);	// only PCM format
					Usb_ack_control_in_ready_send();
					while (!Is_usb_control_out_received());
					Usb_ack_control_out_received_free();
					return TRUE;
				} else return FALSE;

			} else if (type == OUT_CL_INTERFACE){		// set controls
				if (wValue_msb == AUDIO_AS_ACT_ALT_SETTINGS
					&& request == AUDIO_CS_REQUEST_CUR){
					Usb_ack_setup_received_free();
					while (!Is_usb_control_out_received());
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					usb_alternate_setting_out = Usb_read_endpoint_data(EP_CONTROL, 8);
					usb_alternate_setting_out_changed = TRUE;
					Usb_ack_control_out_received_free();
					Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
					while (!Is_usb_control_in_ready()); //!< waits for status phase done
					return FALSE;
				}
			} // end OUT_CL_INTERFACE
		} // end DSC_INTERFACE_AS_OUT

		if ( (wIndex % 256) == DSC_INTERFACE_AUDIO){// low byte wIndex is Interface number
													// high byte is for EntityID

			if (type == IN_CL_INTERFACE){			// get controls
				switch (wIndex /256){
				case CSD_ID_1:
					if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb == 0
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();

						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[3]); // 0x0000bb80 is 48khz
						Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[2]); // 0x00017700 is 96khz
						Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[1]); // 0x0002ee00 is 192khz
						Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[0]);
						Usb_ack_control_in_ready_send();

						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;
					}
					else if (wValue_msb == AUDIO_CS_CONTROL_CLOCK_VALID && wValue_lsb == 0
							 && request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();

						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						Usb_write_endpoint_data(EP_CONTROL, 8, TRUE);	// always valid
						// temp hack to give total # of bytes requested
						for (i = 0; i < (wLength - 1); i++)
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;
					}
					else if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb == 0
							 && request == AUDIO_CS_REQUEST_RANGE){
						Usb_ack_setup_received_free();

						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						// give total # of bytes requested
						for (i = 0; i < (wLength); i++){
							if (FEATURE_DAC_ES9022)
								Usb_write_endpoint_data(EP_CONTROL, 8, Speedx_1[i]);
							else Usb_write_endpoint_data(EP_CONTROL, 8, Speedx_2[i]);
							}
						Usb_ack_control_in_ready_send();

						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;

					} else return FALSE;

				case CSD_ID_2:
					if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb == 0
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[3]); // 0x0000bb80 is 48khz
						Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[2]); // 0x00017700 is 96khz
						Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[1]); // 0x0002ee00 is 192khz
						Usb_write_endpoint_data(EP_CONTROL, 8, current_freq.freq_bytes[0]);
						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;
					}
					else if (wValue_msb == AUDIO_CS_CONTROL_CLOCK_VALID && wValue_lsb == 0
							 && request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						Usb_write_endpoint_data(EP_CONTROL, 8, TRUE);	// always valid
						// temp hack to give total # of bytes requested
						for (i = 0; i < (wLength - 1); i++)
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);

						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;
					}
					else if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb == 0
							 && request == AUDIO_CS_REQUEST_RANGE){
						Usb_ack_setup_received_free();

						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						// give total # of bytes requested
						for (i = 0; i < (wLength); i++)
							Usb_write_endpoint_data(EP_CONTROL, 8, Speedx_2[i]);
						//							  LED_Toggle(LED0);
						Usb_ack_control_in_ready_send();

						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;



					} else return FALSE;


				case CSX_ID:
					if (wValue_msb == AUDIO_CX_CLOCK_SELECTOR && wValue_lsb == 0
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						Usb_write_endpoint_data(EP_CONTROL, 8, clock_selected);
						// temp hack to give total # of bytes requested
						for (i = 0; i < (wLength - 1); i++)
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;
					} else return FALSE;

				case MIC_FEATURE_UNIT_ID:
					if (wValue_msb == AUDIO_FU_CONTROL_CS_MUTE
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						Usb_write_endpoint_data(EP_CONTROL, 8, mute);
						// temp hack to give total # of bytes requested
						for (i = 0; i < (wLength - 1); i++)
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);

						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;
					} else return FALSE;

				case SPK_FEATURE_UNIT_ID:
					if (wValue_msb == AUDIO_FU_CONTROL_CS_MUTE
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						Usb_write_endpoint_data(EP_CONTROL, 8, spk_mute);
						// temp hack to give total # of bytes requested
						for (i = 0; i < (wLength - 1); i++)
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);

						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;
					} else return FALSE;


				case INPUT_TERMINAL_ID:
					if (wValue_msb == AUDIO_TE_CONTROL_CS_CLUSTER && wValue_lsb == 0
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						if (usb_alternate_setting == 1) {
							Usb_write_endpoint_data(EP_CONTROL, 8, INPUT_TERMINAL_NB_CHANNELS);
							Usb_write_endpoint_data(EP_CONTROL, 8, (U8) INPUT_TERMINAL_CHANNEL_CONF);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, INPUT_TERMINAL_STRING_DESC);
						}
						else {			// zero's at startup alt setting 0
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						};
						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;
					} else return FALSE;

				case SPK_INPUT_TERMINAL_ID:
					if (wValue_msb == AUDIO_TE_CONTROL_CS_CLUSTER && wValue_lsb == 0
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						if (usb_alternate_setting_out == 1) {
							Usb_write_endpoint_data(EP_CONTROL, 8, SPK_INPUT_TERMINAL_NB_CHANNELS);
							Usb_write_endpoint_data(EP_CONTROL, 8, (U8) SPK_INPUT_TERMINAL_CHANNEL_CONF);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, INPUT_TERMINAL_STRING_DESC);
						}
						else {			// zero's at startup alt setting 0
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						};
						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received());
						Usb_ack_control_out_received_free();
						return TRUE;
					} else return FALSE;
				default:
					return FALSE;
				} // end switch EntityID
			} else if (type == OUT_CL_INTERFACE){		// set controls
				switch (wIndex /256){
				case CSD_ID_1:							// set CUR freq
				case CSD_ID_2:
					if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb == 0
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received());
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						current_freq.freq_bytes[3]=Usb_read_endpoint_data(EP_CONTROL, 8);		// read 4 bytes freq to set
						current_freq.freq_bytes[2]=Usb_read_endpoint_data(EP_CONTROL, 8);
						current_freq.freq_bytes[1]=Usb_read_endpoint_data(EP_CONTROL, 8);
						current_freq.freq_bytes[0]=Usb_read_endpoint_data(EP_CONTROL, 8);
						freq_changed = TRUE;
						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready()); //!< waits for status phase done
						return TRUE;
					}
					else return FALSE;
				case CSX_ID:
					if (wValue_msb == AUDIO_CX_CLOCK_SELECTOR && wValue_lsb == 0
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received());
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						clock_selected = Usb_read_endpoint_data(EP_CONTROL, 8);
						clock_changed = TRUE;
						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready()); //!< waits for status phase done
						if (clock_selected < 1 || clock_selected > CSX_INPUT_PINS)
							clock_selected = 1;
						return TRUE;
					} else return FALSE;
				case MIC_FEATURE_UNIT_ID:
					if (wValue_msb == AUDIO_FU_CONTROL_CS_MUTE
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received());
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						mute = Usb_read_endpoint_data(EP_CONTROL, 8);
						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready()); //!< waits for status phase done
						return TRUE;
					} else return FALSE;
				case SPK_FEATURE_UNIT_ID:
					if (wValue_msb == AUDIO_FU_CONTROL_CS_MUTE
						&& request == AUDIO_CS_REQUEST_CUR){
						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received());
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						spk_mute = Usb_read_endpoint_data(EP_CONTROL, 8);
						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready()); //!< waits for status phase done
						return TRUE;
					} else return FALSE;
				default:
					return FALSE;
				}
			} // end OUT_CL_INTERFACE

		} // end Audio Control Interface


	} // end CL_INTERFACE

	return FALSE;  // No supported request
}

#endif  // USB_DEVICE_FEATURE == ENABLED
