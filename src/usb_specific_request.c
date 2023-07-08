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
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "image.h"
#include "usart.h"
#include "pm.h"
#include "Mobo_config.h"
#include "DG8SAQ_cmd.h"
#include "features.h"
// #include "usb_audio.h"
// #include "device_audio_task.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________


//_____ P R I V A T E   D E C L A R A T I O N S ____________________________

U8 usb_feature_report[3];
U8 usb_report[3];

U8 g_u8_report_rate=0;

// S_line_coding   line_coding;
U8 clock_selected = 1;
Bool clock_changed = FALSE;

volatile  U16	usb_interface_nb;
volatile  U8	usb_alternate_setting, usb_alternate_setting_out;
volatile  Bool  usb_alternate_setting_changed, usb_alternate_setting_out_changed;

S_freq current_freq;
Bool freq_changed = FALSE;

// static U8    wValue_msb;
// static U8    wValue_lsb;
// static U16   wIndex;
// static U16   wLength;


extern const    void *pbuffer;
extern          U16   data_to_transfer;



//_____ D E C L A R A T I O N S ____________________________________________


// For lack of a better place, here is the volume control format message.
// Input: 16-bit volume control word from USB, 256*dB
// Output: 32-bit volume multiplier to volume control
S32 usb_volume_format (S16 spk_vol_usb) {
	S32 V = VOL_MULT_UNITY;		// Encodes unity

	if ( (spk_vol_usb < VOL_MIN) || (spk_vol_usb > VOL_MAX) )
		return 0;			// Full mute

	// simple shifts for 6dB steps
	spk_vol_usb -= 0.5*256; 	// One step up to allow for clean loops
	while (spk_vol_usb < (-6 * 256) ) {
		V >>= 1;
		spk_vol_usb += (6 * 256) ;
	}
	// >> 4 is approximately 0.5dB
	// Method misses by -0.79..0.0dB. but we've got clean shifts at all -6dB intervals.
	while (spk_vol_usb < (-0.5 * 256) ) {
		V -= V >> 4;
		spk_vol_usb += (0.5 * 256) ;
	}

	return V;
}

// For lack of a better place to put it... 8-bit RNG for dithering for 32-24 bit quantization
// Source: http://stackoverflow.com/questions/16746971/what-is-the-fastest-way-to-generate-pseudo-random-number-by-8-bit-mcu
uint8_t rand8(void) {
	#define STATE_BYTES 7
	#define MULT 0x13B /* for STATE_BYTES==6 only */
	#define MULT_LO (MULT & 255)
	#define MULT_HI (MULT & 256)
	static uint8_t state[STATE_BYTES] =
	{ 0x87, 0xdd, 0xdc, 0x10, 0x35, 0xbc, 0x5c };
	static uint16_t c = 0x42;
	static int i = 0;
	uint16_t t;
	uint8_t x;

	x = state[i];
	t = (uint16_t)x * MULT_LO + c;
	c = t >> 8;
#if MULT_HI
	c += x;
#endif
	x = t & 255;
	state[i] = x;
	if (++i >= sizeof(state))
		i = 0;
	return x;
}

// Store and retrieve volume control
// FIX: must become significantly faster in order to execute without ticks!
S16 usb_volume_flash(U8 channel, S16 volume, U8 rw) {
	S16 temp;

	if (channel == CH_LEFT) {
		if (rw == VOL_READ) {
			temp = (feature_get_nvram(feature_msb_vol_L) << 8) + feature_get_nvram(feature_lsb_vol_L);
			if ( (temp >= VOL_MIN) && (temp <= VOL_MAX) ) {
				return temp;
			}
			else {
				temp = VOL_DEFAULT;
				feature_set_nvram(feature_msb_vol_L, (U8)(temp >> 8));	// Storing default msb
				feature_set_nvram(feature_lsb_vol_L, (U8)(temp >> 0));	// Storing default lsb
				return temp;
			}
		}
		else if (rw == VOL_WRITE) {
			if ( (volume >= VOL_MIN) && (volume <= VOL_MAX) ) {
				feature_set_nvram(feature_msb_vol_L, (U8)(volume >> 8));
				feature_set_nvram(feature_lsb_vol_L, (U8)(volume >> 0));
				return 1;
			}
			else {
				return 0;
			}
		}
	}
	else if (channel == CH_RIGHT) {
		if (rw == VOL_READ) {
			temp = (feature_get_nvram(feature_msb_vol_R) << 8) + feature_get_nvram(feature_lsb_vol_R);
			if ( (temp >= VOL_MIN) && (temp <= VOL_MAX) )
				return temp;
			else {
				temp = VOL_DEFAULT;
				feature_set_nvram(feature_msb_vol_R, (U8)(temp >> 8));	// Storing default msb
				feature_set_nvram(feature_lsb_vol_R, (U8)(temp >> 0));	// Storing default lsb
				return temp;
			}
		}
		else if (rw == VOL_WRITE) {
			if ( (volume >= VOL_MIN) && (volume <= VOL_MAX) ) {
				feature_set_nvram(feature_msb_vol_R, (U8)(volume >> 8));
				feature_set_nvram(feature_lsb_vol_R, (U8)(volume >> 0));
				return 1;
			}
			else
				return 0;
		}
	}

	return 0;
}

//! @brief This function configures the endpoints of the device application.
//! This function is called when the set configuration request has been received.
//!
void usb_user_endpoint_init(U8 conf_nb)
{
	image_user_endpoint_init(conf_nb);
}

//! @brief This function handles usb_set_interface side effects.
//! This function is called from usb_set_interface in usb_standard_request
//! in case there are side effects of the interface change to be handled.
void usb_user_set_interface(U8 wIndex, U8 wValue) {
	image_user_set_interface(wIndex, wValue);
}

//! This function is called by the standard USB read request function when
//! the USB request is not supported. This function returns TRUE when the
//! request is processed. This function returns FALSE if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard USB read request function.
//!
Bool usb_user_read_request(U8 type, U8 request)
{
  // Test for Vendor specific request - DG8SAQ type of request
  if ( (type & DRT_MASK) == DRT_VENDOR )
	  return usb_user_DG8SAQ (type, request);
  return image_user_read_request(type, request);
}


//! This function returns the size and the pointer on a user information
//! structure
//!
Bool usb_user_get_descriptor(U8 type, U8 string)
{
  pbuffer = NULL;

  switch (type)
  {
  case STRING_DESCRIPTOR:
    switch (string)
    {
    case LANG_ID:
      data_to_transfer = sizeof(usb_user_language_id);
      pbuffer = &usb_user_language_id;
      break;

    case MAN_INDEX:
      data_to_transfer = sizeof(usb_user_manufacturer_string_descriptor);
      pbuffer = &usb_user_manufacturer_string_descriptor;
      break;

    case PROD_INDEX:
      data_to_transfer = sizeof(usb_user_product_string_descriptor);
      pbuffer = &usb_user_product_string_descriptor;
      break;

    case SN_INDEX:
      data_to_transfer = sizeof(usb_user_serial_number);
      pbuffer = &usb_user_serial_number;
      break;

    case CLOCK_SOURCE_1_INDEX:
      data_to_transfer = sizeof(usb_user_clock_source_1);
      pbuffer = &usb_user_clock_source_1;
      break;

    case CLOCK_SOURCE_2_INDEX:
      data_to_transfer = sizeof(usb_user_clock_source_2);
      pbuffer = &usb_user_clock_source_2;
      break;

    case CLOCK_SELECTOR_INDEX:
       data_to_transfer = sizeof(usb_user_clock_selector);
       pbuffer = &usb_user_clock_selector;
       break;

    case WL_INDEX:
       data_to_transfer = sizeof(usb_user_wl);
       pbuffer = &usb_user_wl;
       break;

    case AIT_INDEX:
       data_to_transfer = sizeof(usb_user_ait);
       pbuffer = &usb_user_ait;
       break;

    case AOT_INDEX:
       data_to_transfer = sizeof(usb_user_aot);
       pbuffer = &usb_user_aot;
       break;

    case AIN_INDEX:
       data_to_transfer = sizeof(usb_user_ain);
       pbuffer = &usb_user_ain;
       break;

    case AIA_INDEX:
       data_to_transfer = sizeof(usb_user_aia);
       pbuffer = &usb_user_aia;
       break;

    case LEFT_CH_INDEX:
       data_to_transfer = sizeof(usb_left_channel);
       pbuffer = &usb_left_channel;
       break;

    case RIGHT_CH_INDEX:
       data_to_transfer = sizeof(usb_right_channel);
       pbuffer = &usb_right_channel;
       break;


    default:
      break;
    }
    break;

  default:
    break;
  }

  return pbuffer != NULL;
}

Bool usb_user_DG8SAQ(U8 type, U8 command) {

	U16 wValue, wIndex, wLength;
	U8 replyLen;
	int x;

    // Grab the wValue, wIndex / wLength
	wValue  = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
	wIndex  = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
	wLength  = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));

	//-------------------------------------------------------------------------------
	// Process USB Host to Device transmissions.  No result is returned.
	//-------------------------------------------------------------------------------
	if (type == (DRD_OUT | DRT_STD | DRT_VENDOR)) {
		Usb_ack_setup_received_free();
		while (!Is_usb_control_out_received());
		Usb_reset_endpoint_fifo_access(EP_CONTROL);
		
		// This function is stripped down. Go back in commits to determine original version!

		//for (x = 0; x<wLength;x++)
		if (wLength>0)
			for (x = wLength-1; x>=0;x--) {
				Usb_read_endpoint_data(EP_CONTROL, 8);
			}
		Usb_ack_control_out_received_free();
		Usb_ack_control_in_ready_send();
		while (!Is_usb_control_in_ready());
	}
	//-------------------------------------------------------------------------------
	// Process USB query commands and return a result (flexible size data payload)
	//-------------------------------------------------------------------------------
	else if (type == (DRD_IN | DRT_STD | DRT_VENDOR)) {
		// This is our all important hook - Process and execute command, read CW paddle state etc...
		
		// This function is stripped down. Go back in commits to determine original version!

		replyLen = 1; 

		Usb_ack_setup_received_free();

		Usb_reset_endpoint_fifo_access(EP_CONTROL);

		// Write out if packet is larger than zero
		if (replyLen) {
			for (x = replyLen-1; x>=0;x--) {
				Usb_write_endpoint_data(EP_CONTROL, 8, 0);	// send the reply
			}
		}

		Usb_ack_control_in_ready_send();
		while (!Is_usb_control_in_ready());			// handshake modified by Alex 16 May 2010

		while ( !Is_usb_control_out_received());
		Usb_ack_control_out_received_free();

	}

	return TRUE;
}

#endif  // USB_DEVICE_FEATURE == ENABLED
