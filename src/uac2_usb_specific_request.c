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
#include "pdca.h"
#include "features.h"
#include "Mobo_config.h"
#include "usb_audio.h"
#include "device_audio_task.h"
#include "uac2_device_audio_task.h"
#include "taskAK5394A.h"

//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________


//_____ P R I V A T E   D E C L A R A T I O N S ____________________________


static U8 wValue_msb;
static U8 wValue_lsb;
static U16 wIndex;
static U16 wLength;

extern const void *pbuffer;
extern U16 data_to_transfer;


// Send a descriptor to the Host, if needed by means of multiple fillings of EP0
void send_descriptor(U16 wLength, Bool zlp) {
	if (wLength > data_to_transfer) {
		zlp = !(data_to_transfer % EP_CONTROL_LENGTH); //!< zero length packet condition
	} else {
		data_to_transfer = wLength; //!< send only requested number of data bytes
	}

	Usb_ack_nak_out(EP_CONTROL);

	while (data_to_transfer && (!Is_usb_nak_out(EP_CONTROL))) {
		while (!Is_usb_control_in_ready() && !Is_usb_nak_out(EP_CONTROL))
			;

		if (Is_usb_nak_out(EP_CONTROL))
			break; // don't clear the flag now, it will be cleared after

		Usb_reset_endpoint_fifo_access(EP_CONTROL);
		data_to_transfer = usb_write_ep_txpacket(EP_CONTROL, pbuffer,
				data_to_transfer, &pbuffer);
		if (Is_usb_nak_out(EP_CONTROL))
			break;
		else
			Usb_ack_control_in_ready_send(); //!<  Send data until necessary
	}

	if (zlp && (!Is_usb_nak_out(EP_CONTROL))) {
		while (!Is_usb_control_in_ready())
			;
		Usb_ack_control_in_ready_send();
	}

	while (!(Is_usb_nak_out(EP_CONTROL)))
		;
	Usb_ack_nak_out(EP_CONTROL);
	while (!Is_usb_control_out_received())
		;
	Usb_ack_control_out_received_free();
}

// Sample rate triplet. Windows 10 Creators Update refused. Use single frequencies.
// This will cause the ASIO driver to not see the 196ksps definition.
// To fix -that- search for "triplets" in the ASIO driver code and change the array size from 64 to 128.
//const U8 Speedx[38] = { // 74
	
#ifdef HW_GEN_FMADC				// Hardware is fixed to 96ksps. We could use mobo_srd() and enumerate accordingly, but that messes with the whole USB transmit structure
	const U8 Speedx_hs[14] = {	// sample rate samplerate descriptor speed_hs speed_fs <- Comments to find this place more easily
		0x01, 0x00, // Number of sample rate triplets with UAC2 over USB 1.1 (not tested!)

		0x00,0x77,0x01,0x00,	//96k Min
		0x00,0x77,0x01,0x00,	//96k Max
		0x00,0x00,0x00,0x00,	// 0 Res
	};
	const U8 Speedx_fs[14] = {
		0x01, 0x00, // Number of sample rate triplets with UAC2 over USB 1.1 (not very well tested!)

		0x00,0x77,0x01,0x00,	//96k Min
		0x00,0x77,0x01,0x00,	//96k Max
		0x00,0x00,0x00,0x00,	// 0 Res
	};
#else // Conventional sample rates for UAC2
	const U8 Speedx_hs[74] = {
		0x06, 0x00, // Number of sample rate tripl|ets with UAC2 over USB 2.0

		0x44,0xac,0x00,0x00,	//44.1k Min
		0x44,0xac,0x00,0x00,	//44.1k Max
		0x00,0x00,0x00,0x00,	// 0 Res

		0x80,0xbb,0x00,0x00,	//48k Min
		0x80,0xbb,0x00,0x00,	//48k Max
		0x00,0x00,0x00,0x00,	// 0 Res

		0x88,0x58,0x01,0x00,	//88.2k Min
		0x88,0x58,0x01,0x00,	//88.2k Max
		0x00,0x00,0x00,0x00,	// 0 Res

		0x00,0x77,0x01,0x00,	//96k Min
		0x00,0x77,0x01,0x00,	//96k Max
		0x00,0x00,0x00,0x00,	// 0 Res

		0x10,0xb1,0x02,0x00,	//176.4k Min
		0x10,0xb1,0x02,0x00,	//176.4k Max
		0x00,0x00,0x00,0x00,	// 0 Res

		0x00,0xee,0x02,0x00,	//192k Min
		0x00,0xee,0x02,0x00,	//192k Max
		0x00,0x00,0x00,0x00,	// 0 Res
	};

	const U8 Speedx_fs[26] = {
		0x02, 0x00, // Number of sample rate triplets with UAC2 over USB 1.1 (not very well tested!)

		0x44,0xac,0x00,0x00,	//44.1k Min
		0x44,0xac,0x00,0x00,	//44.1k Max
		0x00,0x00,0x00,0x00,	// 0 Res

		0x80,0xbb,0x00,0x00,	//48k Min
		0x80,0xbb,0x00,0x00,	//48k Max
		0x00,0x00,0x00,0x00,	// 0 Res
	};
#endif // End of sample rate definitions for UAC2




/*
// Sample rate triplet. Works in Win10 build 16232 and in ASIO driver
const U8 Speedx[38] = {
	0x03, 0x00, // Number of sample rate triplets

	0x44,0xac,0x00,0x00,	//44.1k Min
	0x80,0xbb,0x00,0x00,	//48k Max
	0x3c,0x0f,0x00,0x00,	//48-44.1 Res

	0x88,0x58,0x01,0x00,	//88.2k Min
	0x00,0x77,0x01,0x00,	//96k Max
	0x78,0x1e,0x00,0x00,	//96-88.2 Res

	0x10,0xb1,0x02,0x00,	//176.4k Min
	0x00,0xee,0x02,0x00,	//192k Max
	0xf0,0x3c,0x00,0x00,	//192-176.4 Res
};
*/


//_____ D E C L A R A T I O N S ____________________________________________


void uac2_freq_change_handler() {

	if (freq_changed) {

#ifdef HW_GEN_RXMOD
		if (input_select == MOBO_SRC_UAC2) { // Only mute if appropriate. Perhaps input has changed to NONE before this can execute
			spk_mute = TRUE; // mute speaker while changing frequency and oscillator
			mobo_clear_dac_channel();
		}
		if ( (input_select == MOBO_SRC_UAC2) || (input_select == MOBO_SRC_NONE) ) {	// Only change I2S settings if appropriate
			mobo_xo_select(spk_current_freq.frequency, MOBO_SRC_UAC2);	// Give USB the I2S control with proper MCLK
			mobo_clock_division(spk_current_freq.frequency);	// Re-configure correct USB sample rate

			// Will this work if we go from SPDIF to USB already playing at different sample rate?

		}
//		if (input_select == MOBO_SRC_UAC2) {	// Only change I2S settings if appropriate
//			mobo_led_select(spk_current_freq.frequency, MOBO_SRC_UAC2); // GPIO frequency indication on front RGB LED
//		}
#else
		spk_mute = TRUE; // mute speaker while changing frequency and oscillator
		#ifdef USB_STATE_MACHINE_DEBUG
			print_dbg_char_char('=');
		#endif
		mobo_clear_dac_channel();

		mobo_xo_select(spk_current_freq.frequency, MOBO_SRC_UAC2); // GPIO XO control and frequency indication
		mobo_clock_division(spk_current_freq.frequency);
#endif

		/*
		 poolingFreq = 8000 / (1 << (EP_INTERVAL_2_HS - 1));
		 FB_rate_int = spk_current_freq.frequency / poolingFreq;
		 FB_rate_frac = spk_current_freq.frequency % poolingFreq;
		 FB_rate = (FB_rate_int << 16) | (FB_rate_frac << 4);
		 */
		if (spk_current_freq.frequency == FREQ_96) {

#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC) // FMADC_site
			// Avoid when using SSC_RX for SPDIF buffering? 
#else
			pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
			pdca_disable(PDCA_CHANNEL_SSC_RX);
#endif

			/*
			 if (FEATURE_LINUX_QUIRK_ON)
			 FB_rate = (96) << 15;
			 else
			 */

			//				FB_rate = (96) << 14; // Generic OS, supported by linux OS patch...
			FB_rate = (99) << 14; // Needed by Linux, linux-quirk replacement, in initial, not in nominal
			FB_rate_initial = FB_rate; // BSB 20131031 Record FB_rate as it was set by control system
			FB_rate_nominal = ((96) << 14) + FB_NOMINAL_OFFSET; // BSB 20131115 Record FB_rate as it was set by control system
		}

		else if (spk_current_freq.frequency == FREQ_88) {

#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC) // FMADC_site
			// Avoid when using SSC_RX for SPDIF buffering?
#else
			pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
			pdca_disable(PDCA_CHANNEL_SSC_RX);
#endif

			/*
			 if (FEATURE_LINUX_QUIRK_ON)
			 FB_rate = (88 << 15) + (1<<15)/5;
			 else
			 */

			//				FB_rate = (88 << 14) + (1<<14)/5; // Generic code, supported by linux OS patch
			FB_rate = (99 << 14); // Needed by Linux, Linux-quirk replacement, in initial, not in nominal
			FB_rate_initial = FB_rate; // BSB 20131031 Record FB_rate as it was set by control system
			FB_rate_nominal = ((88 << 14) + (1 << 14) / 5) + FB_NOMINAL_OFFSET; // BSB 20131115 Record FB_rate as it was set by control system
		}

		else if (spk_current_freq.frequency == FREQ_176) {

#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC) // FMADC_site
			// Avoid when using SSC_RX for SPDIF buffering?
#else
			pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
			pdca_disable(PDCA_CHANNEL_SSC_RX);
#endif

			FB_rate = (176 << 14) + ((1 << 14) * 4) / 10;
			FB_rate_initial = FB_rate; // BSB 20131031 Record FB_rate as it was set by control system
			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET; // BSB 20131115 Record FB_rate as it was set by control system;
		}

		else if (spk_current_freq.frequency == FREQ_192) {

#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC) // FMADC_site
			// Avoid when using SSC_RX for SPDIF buffering?
#else
			pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
			pdca_disable(PDCA_CHANNEL_SSC_RX);
#endif

			FB_rate = (192) << 14;
			FB_rate_initial = FB_rate; // BSB 20131031 Record FB_rate as it was set by control system
			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET; // BSB 20131115 Record FB_rate as it was set by control system;
		}

		else if (spk_current_freq.frequency == FREQ_48) {

#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC) // FMADC_site
			// Avoid when using SSC_RX for SPDIF buffering?
#else
			pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
			pdca_disable(PDCA_CHANNEL_SSC_RX);
#endif

			FB_rate = (48) << 14;
			FB_rate_initial = FB_rate; // BSB 20131031 Record FB_rate as it was set by control system
			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET; // BSB 20131115 Record FB_rate as it was set by control system;
		}

		else if (spk_current_freq.frequency == FREQ_44) {

#if (defined HW_GEN_RXMOD) || (defined HW_GEN_FMADC) // FMADC_site
			// Avoid when using SSC_RX for SPDIF buffering?
#else
			pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
			pdca_disable(PDCA_CHANNEL_SSC_RX);
#endif

			FB_rate = (44 << 14) + (1 << 14) / 10;
			FB_rate_initial = FB_rate; // BSB 20131031 Record FB_rate as it was set by control system
			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET; // BSB 20131115 Record FB_rate as it was set by control system;
		}

		spk_mute = FALSE;
		// reset freq_changed flag
		freq_changed = FALSE;
	}
}

//! @brief This function configures the endpoints of the device application.
//! This function is called when the set configuration request has been received.
//!
void uac2_user_endpoint_init(U8 conf_nb) {
	if (Is_usb_full_speed_mode()) {
		(void) Usb_configure_endpoint(UAC2_EP_AUDIO_OUT_FB, EP_ATTRIBUTES_3, DIRECTION_IN, EP_SIZE_3_FS, DOUBLE_BANK, 0);
		(void) Usb_configure_endpoint(UAC2_EP_AUDIO_OUT, EP_ATTRIBUTES_2, DIRECTION_OUT, EP_SIZE_2_FS, DOUBLE_BANK, 0);
		
		#ifdef FEATURE_ADC_EXPERIMENTAL
			(void)Usb_configure_endpoint(UAC2_EP_AUDIO_IN, EP_ATTRIBUTES_1, DIRECTION_IN, EP_SIZE_1_FS, DOUBLE_BANK, 0);
		#endif
		
		// BSB 20120720 HID insert attempt begin
		#ifdef FEATURE_HID
			(void) Usb_configure_endpoint(UAC2_EP_HID_TX, EP_ATTRIBUTES_4, DIRECTION_IN, EP_SIZE_4_FS, SINGLE_BANK, 0);
//			(void) Usb_configure_endpoint(UAC2_EP_HID_RX, EP_ATTRIBUTES_5, DIRECTION_OUT, EP_SIZE_5_FS, SINGLE_BANK, 0);
		#endif
		// BSB 20120720 HID insert attempt end
	} else {
		(void) Usb_configure_endpoint(UAC2_EP_AUDIO_OUT_FB, EP_ATTRIBUTES_3, DIRECTION_IN, EP_SIZE_3_HS, DOUBLE_BANK, 0);
		(void) Usb_configure_endpoint(UAC2_EP_AUDIO_OUT, EP_ATTRIBUTES_2, DIRECTION_OUT, EP_SIZE_2_HS, DOUBLE_BANK, 0);

		#ifdef FEATURE_ADC_EXPERIMENTAL
			(void)Usb_configure_endpoint(UAC2_EP_AUDIO_IN, EP_ATTRIBUTES_1, DIRECTION_IN, EP_SIZE_1_HS, DOUBLE_BANK, 0);
		#endif

		// BSB 20120720 HID insert attempt begin
		#ifdef FEATURE_HID
			(void) Usb_configure_endpoint(UAC2_EP_HID_TX, EP_ATTRIBUTES_4, DIRECTION_IN, EP_SIZE_4_HS, SINGLE_BANK, 0);
//			(void) Usb_configure_endpoint(UAC2_EP_HID_RX, EP_ATTRIBUTES_5, DIRECTION_OUT, EP_SIZE_5_HS, SINGLE_BANK, 0);
		#endif
		// BSB 20120720 HID insert attempt end
	}
}

//! @brief This function handles usb_set_interface side effects.
//! This function is called from usb_set_interface in usb_standard_request
//! in case there are side effects of the interface change to be handled.
void uac2_user_set_interface(U8 wIndex, U8 wValue) {
	//* Check whether it is the audio streaming interface and Alternate Setting that is being set
	usb_interface_nb = wIndex;
	if (usb_interface_nb == STD_AS_INTERFACE_OUT) {
		usb_alternate_setting_out = wValue;
		usb_alternate_setting_out_changed = TRUE;
//		print_dbg_char('o');
//		print_dbg_char_hex(wValue);
	}

	#ifdef FEATURE_ADC_EXPERIMENTAL
		else if (usb_interface_nb == STD_AS_INTERFACE_IN) {
			usb_alternate_setting = wValue;
			usb_alternate_setting_changed = TRUE;
//			print_dbg_char('i');
//			print_dbg_char_hex(wValue);
		}
	#endif
}

// BSB 20120720 copy from uac1_usb_specific_request.c insert

static Bool uac2_user_get_interface_descriptor() {

#ifdef FEATURE_HID		// This function relates only to HID reports
	Bool zlp;
	U16 wLength;
	U16 wIndex;
	U8 descriptor_type;
	U8 string_type;
	U16 wInterface;

#ifdef USB_STATE_MACHINE_DEBUG
//	print_dbg_char('a'); // xperia
#endif

	zlp = FALSE; /* no zero length packet */
	string_type = Usb_read_endpoint_data(EP_CONTROL, 8); /* read LSB of wValue    */
	descriptor_type = Usb_read_endpoint_data(EP_CONTROL, 8); /* read MSB of wValue    */
	wInterface = usb_format_usb_to_mcu_data(16,Usb_read_endpoint_data(EP_CONTROL, 16));

	switch (descriptor_type) {
	case HID_DESCRIPTOR:

#ifdef USB_STATE_MACHINE_DEBUG
//		print_dbg_char('b'); // xperia
#endif

		if (wInterface == DSC_INTERFACE_HID) {
#if (USB_HIGH_SPEED_SUPPORT==DISABLED)

#ifdef USB_STATE_MACHINE_DEBUG
//			print_dbg_char('c'); // xperia
#endif

			data_to_transfer = sizeof(uac2_usb_conf_desc_fs.hid);
			pbuffer = (const U8*)&uac2_usb_conf_desc_fs.hid;
			break;
#else

#ifdef USB_STATE_MACHINE_DEBUG
//			print_dbg_char('d'); // xperia
#endif

			if (Is_usb_full_speed_mode()) {
				data_to_transfer = sizeof(uac2_usb_conf_desc_fs.hid);
				pbuffer = (const U8*) &uac2_usb_conf_desc_fs.hid;
			} else {
				data_to_transfer = sizeof(uac2_usb_conf_desc_hs.hid);
				pbuffer = (const U8*) &uac2_usb_conf_desc_hs.hid;
			}
			break;
#endif
		}
		return FALSE;
	case HID_REPORT_DESCRIPTOR:

#ifdef USB_STATE_MACHINE_DEBUG
//		print_dbg_char('e'); // xperia
#endif

		//? Why doesn't this test for wInterface == DSC_INTERFACE_HID ?
		data_to_transfer = sizeof(usb_hid_report_descriptor);
		pbuffer = usb_hid_report_descriptor;
		break;
	case HID_PHYSICAL_DESCRIPTOR:

#ifdef USB_STATE_MACHINE_DEBUG
//		print_dbg_char('f'); // xperia
#endif
		// TODO
		return FALSE;
	default:

#ifdef USB_STATE_MACHINE_DEBUG
//		print_dbg_char('g'); // xperia
#endif

		return FALSE;
	}

	wIndex = Usb_read_endpoint_data(EP_CONTROL, 16);
	wIndex = usb_format_usb_to_mcu_data(16, wIndex);
	wLength = Usb_read_endpoint_data(EP_CONTROL, 16);
	wLength = usb_format_usb_to_mcu_data(16, wLength);
	Usb_ack_setup_received_free(); //!< clear the setup received flag
	send_descriptor(wLength, zlp); // Send the descriptor. pbuffer and data_to_transfer are global variables which must be set up by code

#ifdef USB_STATE_MACHINE_DEBUG
//	print_dbg_char('h'); // xperia
#endif

	return TRUE;

#else
	return TRUE;
#endif // FEATURE_HID
}



//! @brief This function manages hid set idle request.
//!
//! @param Duration     When the upper byte of wValue is 0 (zero), the duration is indefinite else from 0.004 to 1.020 seconds
//! @param Report ID    0 the idle rate applies to all input reports, else only applies to the Report ID
//!
void uac2_usb_hid_set_idle(U8 u8_report_id, U8 u8_duration) { // BSB 20120710 prefix "uac2_" added
#ifdef FEATURE_HID
	Usb_ack_setup_received_free();

	if (wIndex == DSC_INTERFACE_HID)
		g_u8_report_rate = u8_duration;

	Usb_ack_control_in_ready_send();
	while (!Is_usb_control_in_ready())
		;
#endif
}

//! @brief This function manages hid get idle request.
//!
//! @param Report ID    0 the idle rate applies to all input reports, else only applies to the Report ID
//!
void uac2_usb_hid_get_idle(U8 u8_report_id) { // BSB 20120710 prefix "uac2_" added
#ifdef FEATURE_HID
	Usb_ack_setup_received_free();

	if ((wLength != 0) && (wIndex == DSC_INTERFACE_HID)) {
		Usb_write_endpoint_data(EP_CONTROL, 8, g_u8_report_rate);
		Usb_ack_control_in_ready_send();
	}

	while (!Is_usb_control_out_received())
		;
	Usb_ack_control_out_received_free();
#endif
}

// BSB 20120720 copy from uac2_usb_specific_request.c end

//! This function is called by the standard USB read request function when
//! the USB request is not supported. This function returns TRUE when the
//! request is processed. This function returns FALSE if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard USB read request function.
//!
Bool uac2_user_read_request(U8 type, U8 request) {
	int i;
	uint8_t temp1 = 0;
	uint8_t temp2 = 0;

/*
#ifdef USB_STATE_MACHINE_DEBUG
	print_dbg_char('t'); // xperia
	print_dbg_char_hex(type); // xperia
	print_dbg_char_hex(request); // xperia
#endif
*/
	// BSB 20120720 added
	// this should vector to specified interface handler
	if (type == IN_INTERFACE && request == GET_DESCRIPTOR)
		return uac2_user_get_interface_descriptor();

	// Read wValue
	// why are these file statics?
	wValue_lsb = Usb_read_endpoint_data(EP_CONTROL, 8);
	wValue_msb = Usb_read_endpoint_data(EP_CONTROL, 8);
	wIndex
			= usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
	wLength
			= usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));

/*
#ifdef USB_STATE_MACHINE_DEBUG
	print_dbg_char('w'); // xperia
	print_dbg_char_hex(wValue_lsb); // xperia
	print_dbg_char_hex(wValue_msb); // xperia
	print_dbg_char('v'); // xperia
	print_dbg_char_hex(wIndex / 256); // xperia MSB
	print_dbg_char_hex(wIndex % 256); // xperia LSB
	print_dbg_char_hex(wLength); // xperia
	print_dbg_char('\n'); // xperia
#endif
*/

	// Mute button push
	// R2101.0114 type=OUT_CL_INTERFACE request=1 wIndex = 0x1401
	// RA101.0114 type=IN_CL_INTERFACE request=1 wIndex = 0x1401

	// BSB 20120720 copy from uac1_usb_specific_request begin

	// It may look like the  OS is pulling the RX endpoint...

	//** Specific request from Class HID
	// this should vector to specified interface handler
#ifdef FEATURE_HID
	if (wIndex == DSC_INTERFACE_HID) // Interface number of HID
	{
		if (type == OUT_CL_INTERFACE) // USB_SETUP_SET_CLASS_INTER
		{
			switch (request) {

			case HID_SET_REPORT:
				// The MSB wValue field specifies the Report Type
				// The LSB wValue field specifies the Report ID
				switch (wValue_msb) {
				case HID_REPORT_INPUT:
					// TODO
					break;

				case HID_REPORT_OUTPUT:
					Usb_ack_setup_received_free();
					while (!Is_usb_control_out_received())
						;
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					usb_report[0] = Usb_read_endpoint_data(EP_CONTROL, 8);
					usb_report[1] = Usb_read_endpoint_data(EP_CONTROL, 8);
					Usb_ack_control_out_received_free();
					Usb_ack_control_in_ready_send();
					while (!Is_usb_control_in_ready())
						;
					return TRUE;

				case HID_REPORT_FEATURE:
					Usb_ack_setup_received_free();
					while (!Is_usb_control_out_received())
						;
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					usb_feature_report[0] = Usb_read_endpoint_data(EP_CONTROL, 8);
					usb_feature_report[1] = Usb_read_endpoint_data(EP_CONTROL, 8);
					Usb_ack_control_out_received_free();
					Usb_ack_control_in_ready_send(); //!< send a ZLP for STATUS phase
					while (!Is_usb_control_in_ready())
						; //!< waits for status phase done
					return TRUE;

				}
				break;

			case HID_SET_IDLE:
				uac2_usb_hid_set_idle(wValue_lsb, wValue_msb); // BSB 20120710 prefix "uac2_" added
				return TRUE;

			case HID_SET_PROTOCOL:
				// TODO
				break;
			}
		}
		if (type == IN_CL_INTERFACE) // USB_SETUP_GET_CLASS_INTER
		{
			switch (request) {
			case HID_GET_REPORT:
				switch (wValue_msb) {
				case HID_REPORT_INPUT:
					Usb_ack_setup_received_free();

					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x01); // Hard-coded HID report # 1
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00); // Hard-coded HID no button
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00); // Hard-coded HID no button
//					Usb_write_endpoint_data(EP_CONTROL, 8, usb_report[0]);
//					Usb_write_endpoint_data(EP_CONTROL, 8, usb_report[1]);
					Usb_ack_control_in_ready_send();

					while (!Is_usb_control_out_received())
						;
					Usb_ack_control_out_received_free();
					return TRUE;

				case HID_REPORT_OUTPUT:
					break;

				case HID_REPORT_FEATURE:
					Usb_ack_setup_received_free();

					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, usb_feature_report[0]);
					Usb_write_endpoint_data(EP_CONTROL, 8, usb_feature_report[1]);
					Usb_ack_control_in_ready_send();

					while (!Is_usb_control_out_received())
						;
					Usb_ack_control_out_received_free();
					return TRUE;
				}
				break;
			case HID_GET_IDLE:
				uac2_usb_hid_get_idle(wValue_lsb); // BSB 20120710 prefix "uac2_" added
				return TRUE;
			case HID_GET_PROTOCOL:
				// TODO
				break;
			}
		}
	} // if wIndex ==  HID Interface
#endif

	// BSB 20120720 copy from uac1_usb_specific_request end


	if (type == IN_CL_INTERFACE || type == OUT_CL_INTERFACE) { // process Class Specific Interface

		//  request for AUDIO interfaces

		#ifdef FEATURE_ADC_EXPERIMENTAL // Bringingn ADC back from main branch...
		if (wIndex == DSC_INTERFACE_AS) {				// Audio Streaming Interface
			if (type == IN_CL_INTERFACE) {			// get controls

				if (wValue_msb == AUDIO_AS_VAL_ALT_SETTINGS && wValue_lsb == 0
						&& request == AUDIO_CS_REQUEST_CUR) {
					Usb_ack_setup_received_free();

					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x01);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0b00000011); // alt 0 and 1 valid
					Usb_ack_control_in_ready_send();

					while (!Is_usb_control_out_received())
						;
					Usb_ack_control_out_received_free();
					return TRUE;
				} 
				else if (wValue_msb == AUDIO_AS_ACT_ALT_SETTINGS 
						&& wValue_lsb == 0 && request == AUDIO_CS_REQUEST_CUR) {
					Usb_ack_setup_received_free();
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, usb_alternate_setting);
					Usb_ack_control_in_ready_send();
					while (!Is_usb_control_out_received())
						;
					Usb_ack_control_out_received_free();
					return TRUE;
				} 
				else if (wValue_msb == AUDIO_AS_AUDIO_DATA_FORMAT 
						&& wValue_lsb == 0 && request == AUDIO_CS_REQUEST_CUR) {
					Usb_ack_setup_received_free();
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x01);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);	// only PCM format
					Usb_ack_control_in_ready_send();
					while (!Is_usb_control_out_received())
						;
					Usb_ack_control_out_received_free();
					return TRUE;
				} 
				else 
					return FALSE;
			 } 
			 else if (type == OUT_CL_INTERFACE) {		// set controls
				if (wValue_msb == AUDIO_AS_ACT_ALT_SETTINGS
						&& request == AUDIO_CS_REQUEST_CUR) {
					Usb_ack_setup_received_free();
					while (!Is_usb_control_out_received())
						;
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					usb_alternate_setting = Usb_read_endpoint_data(EP_CONTROL, 8);
					usb_alternate_setting_changed = TRUE;
					Usb_ack_control_out_received_free();
					Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
					while (!Is_usb_control_in_ready())
						; //!< waits for status phase done
					return FALSE;
				}
			} // end OUT_CL_INTERFACE
		} // end DSC_INTERFACE_AS
		#endif // ADC code brought back

		if (wIndex == DSC_INTERFACE_AS_OUT) { // Playback Audio Streaming Interface

			if (type == IN_CL_INTERFACE) { // get controls

				if (wValue_msb == AUDIO_AS_VAL_ALT_SETTINGS && wValue_lsb == 0
						&& request == AUDIO_CS_REQUEST_CUR) {
					Usb_ack_setup_received_free();

					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x01);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0b00000011); // alt 0 and 1 valid
					Usb_ack_control_in_ready_send();

					while (!Is_usb_control_out_received())
						;
					Usb_ack_control_out_received_free();
					return TRUE;
				} 
				else if (wValue_msb == AUDIO_AS_ACT_ALT_SETTINGS
						&& wValue_lsb == 0 && request == AUDIO_CS_REQUEST_CUR) {
					Usb_ack_setup_received_free();
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, usb_alternate_setting_out); // bBitResolution not touched
					Usb_ack_control_in_ready_send();
					while (!Is_usb_control_out_received())
						;
					Usb_ack_control_out_received_free();
					return TRUE;
				} 
				else if (wValue_msb == AUDIO_AS_AUDIO_DATA_FORMAT
						&& wValue_lsb == 0 && request == AUDIO_CS_REQUEST_CUR) {
					Usb_ack_setup_received_free();
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x01);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
					Usb_write_endpoint_data(EP_CONTROL, 8, 0x00); // only PCM format
					Usb_ack_control_in_ready_send();
					while (!Is_usb_control_out_received())
						;
					Usb_ack_control_out_received_free();
					return TRUE;
				} else
					return FALSE;
			} 
			else if (type == OUT_CL_INTERFACE) { // set controls
				if (wValue_msb == AUDIO_AS_ACT_ALT_SETTINGS 
						&& request == AUDIO_CS_REQUEST_CUR) {
					Usb_ack_setup_received_free();
					while (!Is_usb_control_out_received())
						;
					Usb_reset_endpoint_fifo_access(EP_CONTROL);
					usb_alternate_setting_out = Usb_read_endpoint_data(EP_CONTROL, 8); // bBitResolution not touched
					usb_alternate_setting_out_changed = TRUE;
					Usb_ack_control_out_received_free();
					Usb_ack_control_in_ready_send();  //!< send a ZLP for STATUS phase
					while (!Is_usb_control_in_ready())
						; //!< waits for status phase done
					return FALSE;
				}
			} // end OUT_CL_INTERFACE
		} // end DSC_INTERFACE_AS_OUT

		if ((wIndex % 256) == DSC_INTERFACE_AUDIO) { // low byte wIndex is Interface number
			// high byte is for EntityID
			if (type == IN_CL_INTERFACE) { // get controls
				switch (wIndex / 256) {
				case CSD_ID_1:
					if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ //&& wValue_lsb == 0
							&& request == AUDIO_CS_REQUEST_CUR) {
						Usb_ack_setup_received_free();

						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						Usb_write_endpoint_data(EP_CONTROL, 8, spk_current_freq.freq_bytes[3]); // 0x0000bb80 is 48khz
						Usb_write_endpoint_data(EP_CONTROL, 8, spk_current_freq.freq_bytes[2]); // 0x00017700 is 96khz
						Usb_write_endpoint_data(EP_CONTROL, 8, spk_current_freq.freq_bytes[1]); // 0x0002ee00 is 192khz
						Usb_write_endpoint_data(EP_CONTROL, 8, spk_current_freq.freq_bytes[0]);
						Usb_ack_control_in_ready_send();

						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					} else if (wValue_msb == AUDIO_CS_CONTROL_CLOCK_VALID //&& wValue_lsb == 0
							&& request == AUDIO_CS_REQUEST_CUR) {
						Usb_ack_setup_received_free();

						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						Usb_write_endpoint_data(EP_CONTROL, 8, TRUE); // always valid
						// temp hack to give total # of bytes requested
						for (i = 0; i < (wLength - 1); i++)
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					} else if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ //&& wValue_lsb == 0
							&& request == AUDIO_CS_REQUEST_RANGE) {
						Usb_ack_setup_received_free();

						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						if (Is_usb_full_speed_mode()) { // UAC2 over USB 1.1 Not tested
							data_to_transfer = sizeof(Speedx_fs);
							pbuffer = (const U8*)Speedx_fs;
							send_descriptor(wLength, FALSE); // Send the descriptor. pbuffer and data_to_transfer are global variables which must be set up by code
						}
						else { // UAC2 over USB 2.0
							data_to_transfer = sizeof(Speedx_hs);
							pbuffer = (const U8*)Speedx_hs;
							send_descriptor(wLength, FALSE); // Send the descriptor. pbuffer and data_to_transfer are global variables which must be set up by code
						}

						Usb_ack_control_in_ready_send();

						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;

					} else
						return FALSE;

				case CSD_ID_2:
					if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ //&& wValue_lsb == 0
							&& request == AUDIO_CS_REQUEST_CUR) {

#ifdef USB_STATE_MACHINE_DEBUG
						print_dbg_char('k'); // BSB debug 20120910 Xperia
#endif

						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						Usb_write_endpoint_data(EP_CONTROL, 8, spk_current_freq.freq_bytes[3]); // 0x0000bb80 is 48khz
						Usb_write_endpoint_data(EP_CONTROL, 8, spk_current_freq.freq_bytes[2]); // 0x00017700 is 96khz
						Usb_write_endpoint_data(EP_CONTROL, 8, spk_current_freq.freq_bytes[1]); // 0x0002ee00 is 192khz
						Usb_write_endpoint_data(EP_CONTROL, 8, spk_current_freq.freq_bytes[0]);
						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					} else if (wValue_msb == AUDIO_CS_CONTROL_CLOCK_VALID //&& wValue_lsb == 0
							&& request == AUDIO_CS_REQUEST_CUR) {

// #ifdef USB_STATE_MACHINE_DEBUG
//						print_dbg_char('i'); // BSB debug 20120910 Xperia
// #endif

						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						Usb_write_endpoint_data(EP_CONTROL, 8, TRUE); // always valid
						// temp hack to give total # of bytes requested
						for (i = 0; i < (wLength - 1); i++)
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);

						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					} else if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ //&& wValue_lsb == 0
							&& request == AUDIO_CS_REQUEST_RANGE) {

// #ifdef USB_STATE_MACHINE_DEBUG
//						print_dbg_char('j'); // BSB debug 20120910 Xperia
// #endif

						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						if (Is_usb_full_speed_mode()) { // UAC2 over USB 1.1 Not tested
							data_to_transfer = sizeof(Speedx_fs);
							pbuffer = (const U8*)Speedx_fs;
							send_descriptor(wLength, FALSE); // Send the descriptor. pbuffer and data_to_transfer are global variables which must be set up by code
						}
						else { // UAC2 over USB 2.0
							data_to_transfer = sizeof(Speedx_hs);
							pbuffer = (const U8*)Speedx_hs;
							send_descriptor(wLength, FALSE); // Send the descriptor. pbuffer and data_to_transfer are global variables which must be set up by code
						}

						return TRUE;
					} else
						return FALSE;

#ifdef	FEATURE_CLOCK_SELECTOR
				case CSX_ID:
					if (wValue_msb == AUDIO_CX_CLOCK_SELECTOR //&& wValue_lsb == 0
							&& request == AUDIO_CS_REQUEST_CUR) {
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						Usb_write_endpoint_data(EP_CONTROL, 8, clock_selected);
						// temp hack to give total # of bytes requested
						for (i = 0; i < (wLength - 1); i++)
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					} else
						return FALSE;
#endif						

/*
// mic_feature_unit removed from code here
				case MIC_FEATURE_UNIT_ID:
					if ((wValue_msb == AUDIO_FU_CONTROL_CS_MUTE) && (request
							== AUDIO_CS_REQUEST_CUR)) {
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						Usb_write_endpoint_data(EP_CONTROL, 8, mute);
						// temp hack to give total # of bytes requested
						for (i = 0; i < (wLength - 1); i++)
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);

						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					} else
						return FALSE;
*/

#ifdef FEATURE_VOLUME_CTRL
				case SPK_FEATURE_UNIT_ID:

					if ((wValue_msb == AUDIO_FU_CONTROL_CS_MUTE) && (request
							== AUDIO_CS_REQUEST_CUR)) {
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						if (wLength == 1)
							Usb_write_endpoint_data(EP_CONTROL, 8, usb_spk_mute);
						else if (wLength > 1) { // Linux seems to require 2 bytes of mute control
							Usb_write_endpoint_data(EP_CONTROL, 8, usb_spk_mute);
							for (i = 0; i < (wLength - 1); i++)
								Usb_write_endpoint_data(EP_CONTROL, 8, usb_spk_mute); // or 0
						}

//						print_dbg_char('m'); // bBitResolution

#ifdef USB_STATE_MACHINE_DEBUG
						// Trying to catch mute event
//						print_dbg_char('m');
//						print_dbg_char_hex(usb_spk_mute);
//						print_dbg_char(' ');
#endif

						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					}

					// this is like audio_get_cur() for volume but on UAC2
					else if ((wValue_msb == AUDIO_FU_CONTROL_CS_VOLUME)
							&& (request == AUDIO_CS_REQUEST_CUR)) {
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						if (wLength == 2) {
							if (wValue_lsb == CH_LEFT) {
								// Be on the safe side here, even though fetch is done in uac1_device_audio_task.c init
								if (spk_vol_usb_L == VOL_INVALID) {
									// Without working volume flash:
									spk_vol_usb_L = VOL_DEFAULT;
									// With working volume flash:
									// spk_vol_usb_L = usb_volume_flash(CH_LEFT, 0, VOL_READ);
									spk_vol_mult_L = usb_volume_format(
											spk_vol_usb_L);
								}
								Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, spk_vol_usb_L));

/*
#ifdef USB_STATE_MACHINE_DEBUG
								print_dbg_char('g');
								print_dbg_char('L');
								print_dbg_char_hex(((spk_vol_usb_L >> 8) & 0xff));
								print_dbg_char_hex(((spk_vol_usb_L >> 0) & 0xff));
								print_dbg_char('\n');
#endif
*/

							} else if (wValue_lsb == CH_RIGHT) {
								// Be on the safe side here, even though fetch is done in uac1_device_audio_task.c init
								if (spk_vol_usb_R == VOL_INVALID) {
									// Without working volume flash:
									spk_vol_usb_R = VOL_DEFAULT;
									// With working volume flash:
									// spk_vol_usb_R = usb_volume_flash(CH_RIGHT, 0, VOL_READ);
									spk_vol_mult_R = usb_volume_format(
											spk_vol_usb_R);
								}
								Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, spk_vol_usb_R));
							}
						}

						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					}

					// Is (wValue_msb == AUDIO_FU_CONTROL_CS_VOLUME) sane here?
					if ((wValue_msb == AUDIO_FU_CONTROL_CS_VOLUME) && (request
							== AUDIO_CS_REQUEST_RANGE)) {
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						if (wLength == 8) {
//							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_RES));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, 1));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MIN));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MAX));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_RES));
						}

						// Is this ever used? Some weird Linux state? Filling it with 0x10 entries confused Win10 in UAC2
						// Could it be a request for UP TO 0x10 bytes, and that serving one triplet (0x08) is OK?
						// or should we send two triplets, one for L and one for R? N triplets take up 2(N+1) bytes
						// This code works on Win10/UAC2 Todo: Linux, Android, Mac
						else if (wLength == 0x10) {
							// Same as above
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, 1));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MIN));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MAX));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_RES));
/*
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, 2));	// Two triplets
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MIN));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MAX));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_RES));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MIN));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MAX));
							Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_RES));
*/
						}

						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					}

					else
						return FALSE;
#endif 

				case INPUT_TERMINAL_ID:
					if (wValue_msb == AUDIO_TE_CONTROL_CS_CLUSTER //&& wValue_lsb == 0
							&& request == AUDIO_CS_REQUEST_CUR) {
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						if (usb_alternate_setting == 1) {
							Usb_write_endpoint_data(EP_CONTROL, 8, INPUT_TERMINAL_NB_CHANNELS);
							Usb_write_endpoint_data(EP_CONTROL, 8, (U8) INPUT_TERMINAL_CHANNEL_CONF);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, INPUT_TERMINAL_STRING_DESC);
						} else { // zero's at startup alt setting 0
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						};
						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					} else
						return FALSE;

				case SPK_INPUT_TERMINAL_ID:
					if (wValue_msb == AUDIO_TE_CONTROL_CS_CLUSTER //&& wValue_lsb == 0
							&& request == AUDIO_CS_REQUEST_CUR) {
						Usb_ack_setup_received_free();
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						if (usb_alternate_setting_out >= 1) { // bBitResolution
//						if (usb_alternate_setting_out == 1) {
							Usb_write_endpoint_data(EP_CONTROL, 8, SPK_INPUT_TERMINAL_NB_CHANNELS);
							Usb_write_endpoint_data(EP_CONTROL, 8, (U8) SPK_INPUT_TERMINAL_CHANNEL_CONF);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, SPK_INPUT_TERMINAL_STRING_DESC);
						} else { // zero's at startup alt setting 0
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
							Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
						};
						Usb_ack_control_in_ready_send();
						while (!Is_usb_control_out_received())
							;
						Usb_ack_control_out_received_free();
						return TRUE;
					} else
						return FALSE;

				default:
					return FALSE;
				} // end switch EntityID
			}
			else if (type == OUT_CL_INTERFACE) { // set controls
				switch (wIndex / 256) {
				case CSD_ID_1: // set CUR freq of Mic - UNUSED clock generator!
					if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb
							== 0 && request == AUDIO_CS_REQUEST_CUR) {
						freq_changed = TRUE;
						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received())
							;
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						spk_current_freq.freq_bytes[3]
								=Usb_read_endpoint_data(EP_CONTROL, 8); // read 4 bytes freq to set
						spk_current_freq.freq_bytes[2]
								=Usb_read_endpoint_data(EP_CONTROL, 8);
						spk_current_freq.freq_bytes[1]
								=Usb_read_endpoint_data(EP_CONTROL, 8);
						spk_current_freq.freq_bytes[0]
								=Usb_read_endpoint_data(EP_CONTROL, 8);
						uac2_freq_change_handler();

						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send(); //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready())
							; //!< waits for status phase done
						return TRUE;
					} else
						return FALSE;

				case CSD_ID_2: // set CUR freq - Actual clock generator, merge with above code! ADC_site
					if (wValue_msb == AUDIO_CS_CONTROL_SAM_FREQ && wValue_lsb
							== 0 && request == AUDIO_CS_REQUEST_CUR) {
						freq_changed = TRUE;
						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received())
							;
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						spk_current_freq.freq_bytes[3]
								=Usb_read_endpoint_data(EP_CONTROL, 8); // read 4 bytes freq to set
						spk_current_freq.freq_bytes[2]
								=Usb_read_endpoint_data(EP_CONTROL, 8);
						spk_current_freq.freq_bytes[1]
								=Usb_read_endpoint_data(EP_CONTROL, 8);
						spk_current_freq.freq_bytes[0]
								=Usb_read_endpoint_data(EP_CONTROL, 8);
						uac2_freq_change_handler();
						
						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send(); //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready())
							; //!< waits for status phase done
						return TRUE;
					} else
						return FALSE;
						
#ifdef FEATURE_CLOCK_SELECTOR
				case CSX_ID:
					if ((wValue_msb == AUDIO_CX_CLOCK_SELECTOR) && (wValue_lsb
							== 0) && (request == AUDIO_CS_REQUEST_CUR)) {
						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received())
							;
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						clock_selected = Usb_read_endpoint_data(EP_CONTROL, 8);
						clock_changed = TRUE;
						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send(); //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready())
							; //!< waits for status phase done
						if (clock_selected < 1 || clock_selected > CSX_INPUT_PINS)
							clock_selected = 1;
						return TRUE;
					} else
						return FALSE;
#endif						

/*
// mic_feature_unit removed from code here
				case MIC_FEATURE_UNIT_ID:
					if ((wValue_msb == AUDIO_FU_CONTROL_CS_MUTE) && (request
							== AUDIO_CS_REQUEST_CUR)) {
						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received())
							;
						Usb_reset_endpoint_fifo_access(EP_CONTROL);
						mute = Usb_read_endpoint_data(EP_CONTROL, 8);
						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send(); //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready())
							; //!< waits for status phase done
						return TRUE;
					} else
						return FALSE;
*/						

#ifdef FEATURE_VOLUME_CTRL
				case SPK_FEATURE_UNIT_ID:

					if ((wValue_msb == AUDIO_FU_CONTROL_CS_MUTE) && (request
							== AUDIO_CS_REQUEST_CUR)) {

						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received())
							;
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						if (wLength == 1) {
							temp1 = Usb_read_endpoint_data(EP_CONTROL, 8);
						} else if (wLength > 1) {
							for (i = 0; i < (wLength); i++) {
								temp1 = Usb_read_endpoint_data(EP_CONTROL, 8);
							}
						}
						usb_spk_mute = temp1;

						print_dbg_char('M'); // bBitResolution


#ifdef USB_STATE_MACHINE_DEBUG
						// Trying to catch Win10 mute event
						print_dbg_char('M');
						print_dbg_char_hex(usb_spk_mute);
						print_dbg_char(' ');
#endif


						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send(); //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready())
							; //!< waits for status phase done
						return TRUE;
					}

					// This is like audio_set_cur for volume but on UAC2
					else if ((wValue_msb == AUDIO_FU_CONTROL_CS_VOLUME)
							&& (request == AUDIO_CS_REQUEST_CUR)) {
						Usb_ack_setup_received_free();
						while (!Is_usb_control_out_received())
							;
						Usb_reset_endpoint_fifo_access(EP_CONTROL);

						if (wLength == 2) {
							temp1 = Usb_read_endpoint_data(EP_CONTROL, 8);
							temp2 = Usb_read_endpoint_data(EP_CONTROL, 8);
							if (wValue_lsb == CH_LEFT) {
								LSB( spk_vol_usb_L) = temp1;
								MSB( spk_vol_usb_L) = temp2;
								spk_vol_mult_L = usb_volume_format(
										spk_vol_usb_L);
/*
#ifdef USB_STATE_MACHINE_DEBUG
								print_dbg_char('s');
								print_dbg_char('L');
								print_dbg_char_hex(((spk_vol_usb_L >> 8) & 0xff));
								print_dbg_char_hex(((spk_vol_usb_L >> 0) & 0xff));
								print_dbg_char('\n');
#endif
*/
							} else if (wValue_lsb == CH_RIGHT) {
								LSB( spk_vol_usb_R) = temp1;
								MSB( spk_vol_usb_R) = temp2;
								spk_vol_mult_R = usb_volume_format(
										spk_vol_usb_R);
							}
						}

						Usb_ack_control_out_received_free();
						Usb_ack_control_in_ready_send(); //!< send a ZLP for STATUS phase
						while (!Is_usb_control_in_ready())
							; //!< waits for status phase done
						return TRUE;
					}

					else
						return FALSE;
#endif

				default:
					return FALSE;
				}
			} // end OUT_CL_INTERFACE

		} // end Audio Control Interface


	} // end CL_INTERFACE

	return FALSE; // No supported request
}

#endif  // USB_DEVICE_FEATURE == ENABLED
