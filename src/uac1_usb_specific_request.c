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
#include "uac1_usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "usart.h"
#include "pm.h"
#include "pdca.h"
#include "Mobo_config.h"
#include "usb_audio.h"
#include "device_audio_task.h"
#include "uac1_device_audio_task.h"
#include "taskAK5394A.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

/* volume definition moved to usb_specific_request.h (for lack of a better place)
#define VOL_MIN      (S16)0x8000 // only allowed for CUR
#define VOL_MAX      (S16)0x7Fff
#define VOL_RES      0x000A
*/


//_____ P R I V A T E   D E C L A R A T I O N S ____________________________

// U8 usb_feature_report[3];
// U8 usb_report[3];

// U8 g_u8_report_rate=0;

// S_line_coding   line_coding;

// S_freq current_freq;
// Bool freq_changed = FALSE;

static U8    usb_type;
static U8    wValue_msb;
static U8    wValue_lsb;
static U16   wIndex;
static U16   wLength;

static U8			speed = 1;		// speed == 0, sample rate = 44.1khz
									// speed == 1, sample rate = 48khz

extern const    void *pbuffer;
extern          U16   data_to_transfer;

//_____ D E C L A R A T I O N S ____________________________________________


//! @brief This function configures the endpoints of the device application.
//! This function is called when the set configuration request has been received.
//!
void uac1_user_endpoint_init(U8 conf_nb)
{
	if (Is_usb_full_speed_mode()){
		#ifdef FEATURE_HID
			(void)Usb_configure_endpoint(UAC1_EP_HID_TX, EP_ATTRIBUTES_1, DIRECTION_IN, EP_SIZE_1_FS, SINGLE_BANK, 0);
//			(void)Usb_configure_endpoint(UAC1_EP_HID_RX, EP_ATTRIBUTES_2, DIRECTION_OUT, EP_SIZE_2_FS, SINGLE_BANK, 0);
		#endif
		(void)Usb_configure_endpoint(UAC1_EP_AUDIO_OUT, EP_ATTRIBUTES_3, DIRECTION_OUT, EP_SIZE_3_FS, DOUBLE_BANK, 0);
		// BSB 20130604 disabling UAC1 IN		(void)Usb_configure_endpoint(UAC1_EP_AUDIO_IN, EP_ATTRIBUTES_4, DIRECTION_IN, EP_SIZE_4_FS, DOUBLE_BANK, 0);
		(void)Usb_configure_endpoint(UAC1_EP_AUDIO_OUT_FB, EP_ATTRIBUTES_5, DIRECTION_IN, EP_SIZE_5_FS, DOUBLE_BANK, 0);
	} else {
		#ifdef FEATURE_HID
			(void)Usb_configure_endpoint(UAC1_EP_HID_TX, EP_ATTRIBUTES_1, DIRECTION_IN, EP_SIZE_1_HS, SINGLE_BANK, 0);
//			(void)Usb_configure_endpoint(UAC1_EP_HID_RX, EP_ATTRIBUTES_2, DIRECTION_OUT, EP_SIZE_2_HS, SINGLE_BANK, 0);
		#endif
		(void)Usb_configure_endpoint(UAC1_EP_AUDIO_OUT, EP_ATTRIBUTES_3, DIRECTION_OUT, EP_SIZE_3_HS, DOUBLE_BANK, 0);
		// BSB 20130604 disabling UAC1 IN		(void)Usb_configure_endpoint(UAC1_EP_AUDIO_IN, EP_ATTRIBUTES_4, DIRECTION_IN, EP_SIZE_4_HS, DOUBLE_BANK, 0);
		(void)Usb_configure_endpoint(UAC1_EP_AUDIO_OUT_FB, EP_ATTRIBUTES_5, DIRECTION_IN, EP_SIZE_5_HS, DOUBLE_BANK, 0);
	}
}

//! @brief This function handles usb_set_interface side effects.
//! This function is called from usb_set_interface in usb_standard_request
//! in case there are side effects of the interface change to be handled.
void uac1_user_set_interface(U8 wIndex, U8 wValue) {
	//* Check whether it is the audio streaming interface and Alternate Setting that is being set
	usb_interface_nb = wIndex;
	if (usb_interface_nb == STD_AS_INTERFACE_OUT){
		usb_alternate_setting_out = wValue;
		usb_alternate_setting_out_changed = TRUE;
	}

	// BSB 20130604 disabling UAC1 IN
	/*
	else if (usb_interface_nb == STD_AS_INTERFACE_IN) {
		usb_alternate_setting = wValue;
		usb_alternate_setting_changed = TRUE;
	} */
}

static Bool uac1_user_get_interface_descriptor() {
#ifdef FEATURE_HID		// This function relates only to HID reports

	Bool    zlp;
	U16     wLength;
	U16		wIndex;
	U8      descriptor_type;
	U8      string_type;
	U16		wInterface;

#ifdef USB_STATE_MACHINE_DEBUG
//	print_dbg_char('a'); // BSB debug 20120803
#endif

	zlp             = FALSE;                                  /* no zero length packet */
	string_type     = Usb_read_endpoint_data(EP_CONTROL, 8);  /* read LSB of wValue    */
	descriptor_type = Usb_read_endpoint_data(EP_CONTROL, 8);  /* read MSB of wValue    */
	wInterface = usb_format_usb_to_mcu_data(16,Usb_read_endpoint_data(EP_CONTROL, 16));
	switch( descriptor_type ) {
	case HID_DESCRIPTOR:

		if (wInterface == DSC_INTERFACE_HID) {
#if (USB_HIGH_SPEED_SUPPORT==DISABLED)

#ifdef USB_STATE_MACHINE_DEBUG
//			print_dbg_char('c'); // BSB debug 20120803
#endif

			if (FEATURE_BOARD_WIDGET) {
				data_to_transfer = sizeof(uac1_usb_conf_desc_fs_widget.hid);
				pbuffer          = (const U8*)&uac1_usb_conf_desc_fs_widget.hid;
			} else {
				data_to_transfer = sizeof(uac1_usb_conf_desc_fs.hid);
				pbuffer          = (const U8*)&uac1_usb_conf_desc_fs.hid;
			}
			break;
#else

#ifdef USB_STATE_MACHINE_DEBUG
//			 print_dbg_char('d'); // BSB debug 20120803
#endif

			if (FEATURE_BOARD_WIDGET) {
				if( Is_usb_full_speed_mode() ) {
					data_to_transfer = sizeof(uac1_usb_conf_desc_fs_widget.hid);
					pbuffer          = (const U8*)&uac1_usb_conf_desc_fs_widget.hid;
				} else {
					data_to_transfer = sizeof(uac1_usb_conf_desc_hs_widget.hid);
					pbuffer          = (const U8*)&uac1_usb_conf_desc_hs_widget.hid;
				}
			} else {
				if( Is_usb_full_speed_mode() ) {
					data_to_transfer = sizeof(uac1_usb_conf_desc_fs.hid);
					pbuffer          = (const U8*)&uac1_usb_conf_desc_fs.hid;
				} else {
					data_to_transfer = sizeof(uac1_usb_conf_desc_hs.hid);
					pbuffer          = (const U8*)&uac1_usb_conf_desc_hs.hid;
				}
			}
			break;
#endif
		}
		return FALSE;
	case HID_REPORT_DESCRIPTOR:

#ifdef USB_STATE_MACHINE_DEBUG
//		print_dbg_char('e'); // BSB debug 20120803
#endif
		//? Why doesn't this test for wInterface == DSC_INTERFACE_HID ?
		data_to_transfer = sizeof(usb_hid_report_descriptor);
		pbuffer          = usb_hid_report_descriptor;
		break;
	case HID_PHYSICAL_DESCRIPTOR:

#ifdef USB_STATE_MACHINE_DEBUG
//		print_dbg_char('f'); // BSB debug 20120803
#endif
		// TODO
		return FALSE;
	default:

#ifdef USB_STATE_MACHINE_DEBUG
//		print_dbg_char('g'); // BSB debug 20120803
#endif
		return FALSE;
	}

	wIndex = Usb_read_endpoint_data(EP_CONTROL, 16);
	wIndex = usb_format_usb_to_mcu_data(16, wIndex);
	wLength = Usb_read_endpoint_data(EP_CONTROL, 16);
	wLength = usb_format_usb_to_mcu_data(16, wLength);
	Usb_ack_setup_received_free();                          //!< clear the setup received flag

	if (wLength > data_to_transfer)
		{
			zlp = !(data_to_transfer % EP_CONTROL_LENGTH);  //!< zero length packet condition
		}
	else
		{
			data_to_transfer = wLength; //!< send only requested number of data bytes
		}

	Usb_ack_nak_out(EP_CONTROL);

	while (data_to_transfer && (!Is_usb_nak_out(EP_CONTROL)))
		{
			while( !Is_usb_control_in_ready() && !Is_usb_nak_out(EP_CONTROL) );

			if( Is_usb_nak_out(EP_CONTROL) )
				break;    // don't clear the flag now, it will be cleared after

			Usb_reset_endpoint_fifo_access(EP_CONTROL);
			data_to_transfer = usb_write_ep_txpacket(EP_CONTROL, pbuffer,
													 data_to_transfer, &pbuffer);
			if( Is_usb_nak_out(EP_CONTROL) )
				break;
			else
				Usb_ack_control_in_ready_send();  //!< Send data until necessary
		}

	if ( zlp && (!Is_usb_nak_out(EP_CONTROL)) )
		{
			while (!Is_usb_control_in_ready());
			Usb_ack_control_in_ready_send();
		}

	while (!(Is_usb_nak_out(EP_CONTROL)));
	Usb_ack_nak_out(EP_CONTROL);
	while (!Is_usb_control_out_received());
	Usb_ack_control_out_received_free();

#ifdef USB_STATE_MACHINE_DEBUG
//	print_dbg_char('h'); // BSB debug 20120803
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
void uac1_usb_hid_set_idle (U8 u8_report_id, U8 u8_duration ) { // BSB 20120710 prefix "uac1_" added
#ifdef FEATURE_HID
   Usb_ack_setup_received_free();
  
   if( wIndex == DSC_INTERFACE_HID )
     g_u8_report_rate = u8_duration;
   
   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());
#endif
}


//! @brief This function manages hid get idle request.
//!
//! @param Report ID    0 the idle rate applies to all input reports, else only applies to the Report ID
//!
void uac1_usb_hid_get_idle (U8 u8_report_id) { // BSB 20120710 prefix "uac1_" added
#ifdef FEATURE_HID
	Usb_ack_setup_received_free();
   
   if( (wLength != 0) && (wIndex == DSC_INTERFACE_HID) )
   {
      Usb_write_endpoint_data(EP_CONTROL, 8, g_u8_report_rate);
      Usb_ack_control_in_ready_send();
   }
   
   while (!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
#endif
}

#ifdef FEATURE_VOLUME_CTRL
void audio_get_min(void)
{
   U16 i_unit;  // in wIndex
   U16 length;  // in wLength
   i_unit = (wIndex >> 8);			// wIndex high byte is interface number
   length = wLength;

   Usb_ack_setup_received_free();
   Usb_reset_endpoint_fifo_access(EP_CONTROL);

   if ( i_unit == SPK_FEATURE_UNIT_ID) {

         switch (wValue_msb) {
         case CS_MUTE:
            if( length==1 ) {
               Usb_write_endpoint_data(EP_CONTROL, 8, usb_spk_mute);
            }
            break;
         case CS_VOLUME:
            if( length==2 ) {
               Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MIN));
            }
            break;
         }
      }


   // BSB 20130604 disabling UAC1 IN
   /*
   else if( i_unit==MIC_FEATURE_UNIT_ID )
   {
      switch (wValue_msb)
      {
      case CS_MUTE:
         if( length==1 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 8, mute);
         }
         break;
      case CS_VOLUME:
         if( length==2 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MIN));
         }
         break;
      }
    }
    */

/*
		// 44.1khz min sampling freq
    	Usb_write_endpoint_data(EP_CONTROL, 8, 0x44);
    	Usb_write_endpoint_data(EP_CONTROL, 8, 0xac);
    	Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
*/

   Usb_ack_control_in_ready_send();
   while(!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
}
#endif

#ifdef FEATURE_VOLUME_CTRL
void audio_get_max(void) {
   U16 i_unit;
   U16 length;
   i_unit = (wIndex >> 8);			// wIndex high byte is interface number
   length = wLength;

   Usb_ack_setup_received_free();
   Usb_reset_endpoint_fifo_access(EP_CONTROL);

   if ( i_unit == SPK_FEATURE_UNIT_ID) {
	     switch (wValue_msb) {
	      case CS_MUTE:
	         if( length==1 ) {
	            Usb_write_endpoint_data(EP_CONTROL, 8, usb_spk_mute);
	         }
	         break;
	      case CS_VOLUME:
	         if( length==2 ) {
	            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MAX));
	         }
	         break;
	      }
  }

   // BSB 20130604 disabling UAC1 IN
   /*
   else if( i_unit==MIC_FEATURE_UNIT_ID )
   {
      switch (wValue_msb)
      {
      case CS_MUTE:
         if( length==1 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 8, mute);
         }
         break;
      case CS_VOLUME:
         if( length==2 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_MAX));
         }
         break;
      }
   }
   */

/*
		// 48khz max sampling freq
		Usb_write_endpoint_data(EP_CONTROL, 8, 0x80);
		Usb_write_endpoint_data(EP_CONTROL, 8, 0xbb);
		Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
*/

   Usb_ack_control_in_ready_send();
   while(!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
}
#endif

#ifdef FEATURE_VOLUME_CTRL
void audio_get_res(void)
{
   U16 i_unit;
   U16 length;
   i_unit = (wIndex >> 8);			// wIndex high byte is interface number
   length = wLength;

   Usb_ack_setup_received_free();
   Usb_reset_endpoint_fifo_access(EP_CONTROL);

   if ( i_unit==SPK_FEATURE_UNIT_ID) { // FIX: Something is seriously wrong with the value of i_unit
	     switch (wValue_msb) {
	      case CS_MUTE:
	         if( length==1 ) {
	            Usb_write_endpoint_data(EP_CONTROL, 8, usb_spk_mute);
	         }
	         break;
	      case CS_VOLUME:
	         if( length==2 ) {
	            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_RES));
	         }
	         break;
	      }
   }


   // BSB 20130604 disabling UAC1 IN
   /*
   else if( i_unit==MIC_FEATURE_UNIT_ID )
   {
      switch (wValue_msb)
      {
      case CS_MUTE:
         if( length==1 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 8, mute);
         }
         break;
      case CS_VOLUME:
         if( length==2 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, VOL_RES));
         }
         break;
      }
   }
   */

/*
	// 48000 - 44100 = 3900
   	Usb_write_endpoint_data(EP_CONTROL, 8, 0x3c);
   	Usb_write_endpoint_data(EP_CONTROL, 8, 0x0f);
   	Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
*/

   Usb_ack_control_in_ready_send();
   while(!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
}
#endif

void audio_get_cur(void) {
	U16 i_unit;
	U16 length;
	i_unit = (wIndex >> 8);			// wIndex high byte is interface number
	length = wLength;

	Usb_ack_setup_received_free();
	Usb_reset_endpoint_fifo_access(EP_CONTROL);

	if ((usb_type == USB_SETUP_GET_CLASS_ENDPOINT) && (wValue_msb == UAC_EP_CS_ATTR_SAMPLE_RATE)){
		if (speed == 0) {
			Usb_write_endpoint_data(EP_CONTROL, 8, 0x44);
			Usb_write_endpoint_data(EP_CONTROL, 8, 0xac);
			Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
		}
		else {
			Usb_write_endpoint_data(EP_CONTROL, 8, 0x80);
			Usb_write_endpoint_data(EP_CONTROL, 8, 0xbb);
			Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
		}
	}

   // BSB 20130604 disabling UAC1 IN
   /*
   else if( i_unit==MIC_FEATURE_UNIT_ID )
   {
      switch (wValue_msb)
      {
      case CS_MUTE:
         if( length==1 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 8, mute);
         }
         break;
      case CS_VOLUME:
         if( length==2 )
         {
            Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, volume));
         }
         break;
      }

   }
   */

#ifdef FEATURE_VOLUME_CTRL
	else if (i_unit==SPK_FEATURE_UNIT_ID) {
		switch (wValue_msb) {
			case CS_MUTE:
//		   		if (length == 1) { // Don't lock onto set mute status length 1 byte. Not 100% sure if Linux uses ==1 byte
					Usb_write_endpoint_data(EP_CONTROL, 8, usb_spk_mute);
//				}
			break;
			case CS_VOLUME:
				if (length == 2) {
					if (wValue_lsb == CH_LEFT) {
						// Be on the safe side here, even though fetch is done in uac1_device_audio_task.c init
						if (spk_vol_usb_L == VOL_INVALID) {
							// Without working volume flash:
							spk_vol_usb_L = VOL_DEFAULT;
							// With working volume flash:
							// spk_vol_usb_L = usb_volume_flash(CH_LEFT, 0, VOL_READ);
							spk_vol_mult_L = usb_volume_format(spk_vol_usb_L);
						}
						Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, spk_vol_usb_L));

#ifdef USB_STATE_MACHINE_DEBUG
					   print_dbg_char('g');
					   print_dbg_char('l');
					   print_dbg_char_hex(((spk_vol_usb_L >> 8) & 0xff));
					   print_dbg_char_hex(((spk_vol_usb_L >> 0) & 0xff));
					   print_dbg_char('\n');
#endif

					}
					else if (wValue_lsb == CH_RIGHT) {
						// Be on the safe side here, even though fetch is done in uac1_device_audio_task.c init
						if (spk_vol_usb_R == VOL_INVALID) {
							// Without working volume flash:
							spk_vol_usb_R = VOL_DEFAULT;
							// With working volume flash:
							// spk_vol_usb_R = usb_volume_flash(CH_RIGHT, 0, VOL_READ);
							spk_vol_mult_R = usb_volume_format(spk_vol_usb_R);
						}
						Usb_write_endpoint_data(EP_CONTROL, 16, Usb_format_mcu_to_usb_data(16, spk_vol_usb_R));
					}
				}
				break;
		}
	}
#endif

	Usb_ack_control_in_ready_send();
	while(!Is_usb_control_out_received());
	Usb_ack_control_out_received_free();
}

void audio_set_cur(void)
{
   U16 i_unit;
   U16 length;
   i_unit = (wIndex >> 8);			// wIndex high byte is interface number
   length = wLength;

   Usb_ack_setup_received_free();
   while(!Is_usb_control_out_received());
   Usb_reset_endpoint_fifo_access(EP_CONTROL);

   if ((usb_type == USB_SETUP_SET_CLASS_ENDPOINT) && (wValue_msb == UAC_EP_CS_ATTR_SAMPLE_RATE)){
		if (Usb_read_endpoint_data(EP_CONTROL, 8) == 0x44)
			speed = 0;
		else
			speed = 1;

		freq_changed = TRUE;
		if (speed == 0) {		// 44.1khz

#ifdef USB_STATE_MACHINE_DEBUG
			print_dbg_char('1'); // BSB debug 20121212
#endif

			current_freq.frequency = FREQ_44;
			// BSB 20130602: code section moved here from uac1_device_audio_task.c
			FB_rate = (44 << 14) + (1 << 14)/10;
			FB_rate_initial = FB_rate;							// BSB 20131031 Record FB_rate as it was set by control system
			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET;		// BSB 20131115 Record FB_rate as it was set by control system;
		}
		else {					// 48khz

#ifdef USB_STATE_MACHINE_DEBUG
			print_dbg_char('2'); // BSB debug 20121212
#endif

	   		current_freq.frequency = FREQ_48;
			// BSB 20130602: code section moved here from uac1_device_audio_task.c
			FB_rate = 48 << 14;
			FB_rate_initial = FB_rate;							// BSB 20131031 Record FB_rate as it was set by control system
			FB_rate_nominal = FB_rate + FB_NOMINAL_OFFSET;		// BSB 20131115 Record FB_rate as it was set by control system;
		}

#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20) || (defined HW_GEN_RXMOD)
		if (input_select == MOBO_SRC_UAC1) { // Only mute if appropriate. Perhaps input has changed to NONE before this can execute
			spk_mute = TRUE; // mute speaker while changing frequency and oscillator
			mobo_clear_dac_channel();
		}
		if ( (input_select == MOBO_SRC_UAC1) || (input_select == MOBO_SRC_NONE) ) {	// Only change I2S settings if appropriate
			mobo_xo_select(current_freq.frequency, MOBO_SRC_UAC1);	// Give USB the I2S control with proper MCLK
			mobo_clock_division(current_freq.frequency);	// Re-configure correct USB sample rate

			// Will this work if we go from SPDIF to USB already playing at different sample rate?

		}
#else
		spk_mute = TRUE; // mute speaker while changing frequency and oscillator
		#ifdef USB_STATE_MACHINE_DEBUG
			print_dbg_char_char('=');
		#endif
		mobo_clear_dac_channel();

		mobo_xo_select(current_freq.frequency, MOBO_SRC_UAC1); // GPIO XO control and frequency indication
		mobo_clock_division(current_freq.frequency);	// This is redundant in UAC1, but we attempt restart for good measure!
#endif




   }

   // BSB 20160318 experimenting with mute and playback volume control
#ifdef FEATURE_VOLUME_CTRL
   else if (i_unit == SPK_FEATURE_UNIT_ID ) {
		uint8_t temp1 = 0;
		uint8_t temp2 = 0;

	   if (wValue_msb == CS_MUTE) {
//		   if (length == 1) { // Don't lock onto set mute status length 1 byte. Not 100% sure if Linux uses ==1 byte
			   temp1 = Usb_read_endpoint_data(EP_CONTROL, 8);
			   usb_spk_mute = temp1;

//		   }
	   }
	   else if (wValue_msb == CS_VOLUME) {
		   if (length == 2) {
			   temp1 = Usb_read_endpoint_data(EP_CONTROL, 8);
			   temp2 = Usb_read_endpoint_data(EP_CONTROL, 8);
			   if (wValue_lsb == CH_LEFT) {
				   LSB(spk_vol_usb_L)= temp1;
				   MSB(spk_vol_usb_L)= temp2;
				   spk_vol_mult_L = usb_volume_format(spk_vol_usb_L);

/*
#ifdef USB_STATE_MACHINE_DEBUG
				   print_dbg_char('s');
				   print_dbg_char('L');
				   print_dbg_char_hex(((spk_vol_usb_L >> 8) & 0xff));
				   print_dbg_char_hex(((spk_vol_usb_L >> 0) & 0xff));
				   print_dbg_char('\n');
#endif
*/

			   }
			   else if (wValue_lsb == CH_RIGHT) {
				   LSB(spk_vol_usb_R)= temp1;
				   MSB(spk_vol_usb_R)= temp2;
				   spk_vol_mult_R = usb_volume_format(spk_vol_usb_R);
			   }
		   }
	   }
	}
#endif

   // BSB 20130604 disabling UAC1 IN
   /*
   else if( i_unit==MIC_FEATURE_UNIT_ID )
   {
      switch (wValue_msb)
      {
      case CS_MUTE:
         if( length==1 )
         {
            mute=Usb_read_endpoint_data(EP_CONTROL, 8);
         }
         break;
      case CS_VOLUME:
         if( length==2 )
         {
            LSB(volume)= Usb_read_endpoint_data(EP_CONTROL, 8);
            MSB(volume)= Usb_read_endpoint_data(EP_CONTROL, 8);
         }
         break;
      }
   }
   */


   Usb_ack_control_out_received_free();
   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());
}

//! This function is called by the standard USB read request function when
//! the USB request is not supported. This function returns TRUE when the
//! request is processed. This function returns FALSE if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard USB read request function.
//!
Bool uac1_user_read_request(U8 type, U8 request)
{

	usb_type = type;
#ifdef USB_STATE_MACHINE_DEBUG
//	print_dbg_char('z'); // BSB debug 20121212
#endif

	// this should vector to specified interface handler
	if (type == IN_INTERFACE && request == GET_DESCRIPTOR) return uac1_user_get_interface_descriptor();
	// Read wValue
	// why are these file statics?
	wValue_lsb = Usb_read_endpoint_data(EP_CONTROL, 8);
	wValue_msb = Usb_read_endpoint_data(EP_CONTROL, 8);
	wIndex = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
	wLength = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));

	//** Specific request from Class HID
	// this should vector to specified interface handler
#ifdef FEATURE_HID
	if( wIndex == DSC_INTERFACE_HID )   // Interface number of HID
		{

			if( type == OUT_CL_INTERFACE ) // USB_SETUP_SET_CLASS_INTER
				{
					switch( request )
						{

						case HID_SET_REPORT:
							// The MSB wValue field specifies the Report Type
							// The LSB wValue field specifies the Report ID
							switch (wValue_msb)
								{
								case HID_REPORT_INPUT:
									// TODO
									break;

								case HID_REPORT_OUTPUT:
									Usb_ack_setup_received_free();
									while (!Is_usb_control_out_received());
									Usb_reset_endpoint_fifo_access(EP_CONTROL);
									usb_report[0] = Usb_read_endpoint_data(EP_CONTROL, 8);
									usb_report[1] = Usb_read_endpoint_data(EP_CONTROL, 8);
									Usb_ack_control_out_received_free();
									Usb_ack_control_in_ready_send();
									while (!Is_usb_control_in_ready());
									return TRUE;

								case HID_REPORT_FEATURE:
									Usb_ack_setup_received_free();
									while (!Is_usb_control_out_received());
									Usb_reset_endpoint_fifo_access(EP_CONTROL);
									usb_feature_report[0] = Usb_read_endpoint_data(EP_CONTROL, 8);
									usb_feature_report[1] = Usb_read_endpoint_data(EP_CONTROL, 8);
									Usb_ack_control_out_received_free();
									Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
									while (!Is_usb_control_in_ready()); //!< waits for status phase done
									return TRUE;

								}
							break;

						case HID_SET_IDLE:
							uac1_usb_hid_set_idle(wValue_lsb, wValue_msb);  // BSB 20120710 prefix "uac1_" added
							return TRUE;
   
						case HID_SET_PROTOCOL:
							// TODO
							break;
						}
				}
			if( type == IN_CL_INTERFACE) // USB_SETUP_GET_CLASS_INTER
				{
					switch( request )
						{
						case HID_GET_REPORT:
							switch (wValue_msb)
								{
								case HID_REPORT_INPUT:
									Usb_ack_setup_received_free();

									Usb_reset_endpoint_fifo_access(EP_CONTROL);
									Usb_write_endpoint_data(EP_CONTROL, 8, 0x01); // Hard-coded HID report # 1
									Usb_write_endpoint_data(EP_CONTROL, 8, 0x00); // Hard-coded HID no button
									Usb_write_endpoint_data(EP_CONTROL, 8, 0x00); // Hard-coded HID no button
//									Usb_write_endpoint_data(EP_CONTROL, 8, usb_report[0]);
//									Usb_write_endpoint_data(EP_CONTROL, 8, usb_report[1]);
									Usb_ack_control_in_ready_send();

									while (!Is_usb_control_out_received());
									Usb_ack_control_out_received_free();
									return TRUE;

								case HID_REPORT_OUTPUT:
									break;

								case HID_REPORT_FEATURE:
									Usb_ack_setup_received_free();

									Usb_reset_endpoint_fifo_access(EP_CONTROL);
									Usb_write_endpoint_data(EP_CONTROL, 8, 0x01); // Hard-coded HID report # 1
									Usb_write_endpoint_data(EP_CONTROL, 8, usb_feature_report[0]);
									Usb_write_endpoint_data(EP_CONTROL, 8, usb_feature_report[1]);
									Usb_ack_control_in_ready_send();

									while (!Is_usb_control_out_received());
									Usb_ack_control_out_received_free();
									return TRUE;
								}
							break;
						case HID_GET_IDLE:
							uac1_usb_hid_get_idle(wValue_lsb); // BSB 20120710 prefix "uac1_" added
							return TRUE;
						case HID_GET_PROTOCOL:
							// TODO
							break;
						}
				}
	} // if wIndex ==  HID Interface
#endif


	//  assume all other requests are for AUDIO interface

	switch (request)
	    {
		case BR_REQUEST_SET_CUR:
			audio_set_cur();
			return TRUE;
			// No need to break here !

		case BR_REQUEST_SET_MIN:     //! Set MIN,MAX and RES not supported
		case BR_REQUEST_SET_MAX:
		case BR_REQUEST_SET_RES:
			return FALSE;
			// No need to break here !

		case BR_REQUEST_GET_CUR:
			audio_get_cur();
			return TRUE;
			// No need to break here !

#ifdef FEATURE_VOLUME_CTRL
		case BR_REQUEST_GET_MIN:
			audio_get_min();
			return TRUE;
			// No need to break here !

		case BR_REQUEST_GET_MAX:
			audio_get_max();
			return TRUE;
			// No need to break here !

		case BR_REQUEST_GET_RES:
			audio_get_res();
			return TRUE;
			// No need to break here !
#endif

		default:
			return FALSE;
			// No need to break here !
	    }

	return FALSE;  // No supported request
}

#endif  // USB_DEVICE_FEATURE == ENABLED
