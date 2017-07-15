/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 *
 * Copyright (C) Alex Lee 2010, 2011
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
 * Modified by Alex Lee 20 Feb 2010
 * To enumerate as a USB composite device with 4 interfaces:
 * CDC
 * HID (generic HID interface, compatible with Jan Axelson's generichid.exe test programs
 * DG8SAQ (libusb API compatible interface for implementing DG8SAQ EP0 type of interface)
 * Audio (USB Audio Class V2)
 *
 * For SDR-Widget and SDR-Widget-lite, custom boards based on the AT32UC3A3256
 *
 * See http://code.google.com/p/sdr-widget/
 *
 * Rearranged by Roger Critchlow 21 Feb 2011.
 */

#ifndef _UAC1_DESCRIPTORS_H_
#define _UAC1_DESCRIPTORS_H_


//_____ I N C L U D E S ____________________________________________________

#include "conf_usb.h"
#include "features.h"

#if USB_DEVICE_FEATURE == DISABLED
  #error uac1_descriptors.h is #included although USB_DEVICE_FEATURE is disabled
#endif

#include "usb_standard_request.h"
#include "usb_task.h"
#include "usb_descriptors.h"
#include "hid.h"

//_____ M A C R O S ________________________________________________________


//_____ U S B    D E F I N E S _____________________________________________


// CONFIGURATION
// BSB 20130604 disabling UAC1 IN #define NB_INTERFACE	   5	//!  DG8SAQ, HID, Audio (3)
#define NB_INTERFACE	   4	//!  DG8SAQ, HID, Audio (2) OR 3 // HID, Audio (2)
#define CONF_NB            1	//! Number of this configuration
#define CONF_INDEX         0
#define CONF_ATTRIBUTES    USB_CONFIG_BUSPOWERED  // default is bus-powered
#define MAX_POWER          250  // 500mA for bus-powered operation

//Audio Streaming (AS) interface descriptor
// BSB 20130604 disabling UAC1 IN #define STD_AS_INTERFACE_IN				0x04   // Index of Std AS Interface
#define STD_AS_INTERFACE_OUT			0x03

// IAD for Audio
#define FIRST_INTERFACE1	2
// BSB 20130604 disabling UAC1 IN #define INTERFACE_COUNT1	3
#define INTERFACE_COUNT1	2

// USB DG8SAQ Interface descriptor
#define INTERFACE_NB0			    0
#define ALTERNATE_NB0	            0                  //! The alt setting nb of this interface
#define NB_ENDPOINT0			    0                  //! The number of endpoints this interface has
#define INTERFACE_CLASS0		    NO_CLASS          //! No Class
#define INTERFACE_SUB_CLASS0        NO_SUBCLASS        //! No Subclass
#define INTERFACE_PROTOCOL0    		NO_PROTOCOL		   //! No Protocol
#define INTERFACE_INDEX0       		0

#define DSC_INTERFACE_DG8SAQ		INTERFACE_NB0


// USB HID Interface descriptor
#define INTERFACE_NB1			    1
#define ALTERNATE_NB1	            0                  //! The alt setting nb of this interface
#define NB_ENDPOINT1			    2                  //! The number of endpoints this interface has
#define INTERFACE_CLASS1		    HID_CLASS          //! HID Class
#define INTERFACE_SUB_CLASS1        NO_SUBCLASS        //! No Subclass
#define INTERFACE_PROTOCOL1    		NO_PROTOCOL		   //! No Protocol
#define INTERFACE_INDEX1       		0

#define DSC_INTERFACE_HID			INTERFACE_NB1

// HID descriptor
#define HID_VERSION                 0x0111  //! HID Class Specification release number
#define HID_COUNTRY_CODE            0x00    //! Hardware target country
#define HID_NUM_DESCRIPTORS			0x01    //! Number of HID class descriptors to follow

// USB Endpoint 1 descriptor
#define ENDPOINT_NB_1           (UAC1_EP_HID_TX| MSK_EP_DIR)
#define EP_ATTRIBUTES_1         TYPE_INTERRUPT
#define EP_IN_LENGTH_1_FS       8
#define EP_SIZE_1_FS            EP_IN_LENGTH_1_FS
#define EP_IN_LENGTH_1_HS       8
#define EP_SIZE_1_HS            EP_IN_LENGTH_1_HS
#define EP_INTERVAL_1           5               //! Interrupt polling interval from host

// USB Endpoint 2 descriptor - not used
#define ENDPOINT_NB_2           (UAC1_EP_HID_RX)
#define EP_ATTRIBUTES_2         TYPE_INTERRUPT
#define EP_OUT_LENGTH_2_FS      8
#define EP_SIZE_2_FS            EP_OUT_LENGTH_2_FS
#define EP_OUT_LENGTH_2_HS       8
#define EP_SIZE_2_HS            EP_OUT_LENGTH_2_HS
#define EP_INTERVAL_2           5               //! Interrupt polling interval from host

// Standard Audio Control (AC) interface descriptor
#define INTERFACE_NB2       2
#define ALTERNATE_NB2       0
#define NB_ENDPOINT2        0			     //! No endpoint for AC interface
#define INTERFACE_CLASS2    AUDIO_CLASS  	//! Audio Class
#define INTERFACE_SUB_CLASS2 0x01		     //! Audio_control sub class
#define INTERFACE_PROTOCOL2  0x00		     //! Unused
#define INTERFACE_INDEX2     0

#define DSC_INTERFACE_AUDIO			INTERFACE_NB2

// USB Endpoint 3 descriptor
#define ENDPOINT_NB_3       ( UAC1_EP_AUDIO_OUT )
#define EP_ATTRIBUTES_3     0b00000101      // ISOCHRONOUS ASYNCHRONOUS DATA, EXPLICIT FEEDBACK in other EP
//#define EP_ATTRIBUTES_3     0b00001101      // ISOCHRONOUS SYNCHRONOUS
#define EP_IN_LENGTH_3_HS   294				// 3 bytes * 48 khz * stereo + 6 bytes for add sample
#define EP_IN_LENGTH_3_FS	294
#define EP_SIZE_3_FS		EP_IN_LENGTH_3_FS
#define EP_SIZE_3_HS        EP_IN_LENGTH_3_HS
#define EP_INTERVAL_3_FS	0x01			 // one packet per uframe must set to 1 for ms
#define EP_INTERVAL_3_HS    0x04			 // One packet per 8 uframe
#define EP_REFRESH_3_FS		0x00 //0x00	// was 0x05		 // BSB 20130530 should be 0? Added as #define See USB audio 1.0 specification, Table 4-20: Standard AS Isochronous Audio Data Endpoint.
#define EP_REFRESH_3_HS		0x00 //0x00	// was 0x05		 // BSB 20130530 should be 0? Added as #define
//**** BSB: Is this correct with 0x05?? How about UAC1_EP_AUDIO_OUT_FB ??
#define EP_BSYNC_ADDRESS_3	0x05			 // feedback EP is EP 5
//#define EP_BSYNC_ADDRESS_3	0x04			 // feedback EP is EP 4 - using audio input pipe to sync
//#define EP_BSYNC_ADDRESS_3	0x00

// BSB 20130604 disabling UAC1 IN
/*// USB Endpoint 4 descriptor
#define ENDPOINT_NB_4       ( UAC1_EP_AUDIO_IN | MSK_EP_DIR )
// BSB 20130530 was: #define EP_ATTRIBUTES_4     0b00100101      // ISOCHRONOUS ASYNCHRONOUS IMPLICIT FEEDBACK
#define EP_ATTRIBUTES_4     0b00000101      // ISOCHRONOUS ASYNCHRONOUS DATA

#define EP_IN_LENGTH_4_HS   294				// 3 bytes * 48 khz * stereo + 6 bytes for add sample
#define EP_IN_LENGTH_4_FS	294
#define EP_SIZE_4_FS		EP_IN_LENGTH_4_FS
#define EP_SIZE_4_HS        EP_IN_LENGTH_4_HS
#define EP_INTERVAL_4_FS	0x01			 // one packet per uframe
#define EP_INTERVAL_4_HS    0x04			 // One packet per 8 uframe
*/

/* Note:  The EPs have to be re-arranged.  Feedback EP has to be immediately following the OUT EP
// USB Endpoint 5 descriptor*/
#define ENDPOINT_NB_5       ( UAC1_EP_AUDIO_OUT_FB | MSK_EP_DIR )
#define EP_ATTRIBUTES_5     0b00010001      // ISOCHROUNOUS FEEDBACK // BSB 20131022 is this in correspondance with Table 4-22 of Audio10.PDF? Bit 6 reserved?
#define EP_IN_LENGTH_5_FS   3				// 3 bytes
#define EP_IN_LENGTH_5_HS	4				// 4 bytes
#define EP_SIZE_5_FS		EP_IN_LENGTH_5_FS
#define EP_SIZE_5_HS        EP_IN_LENGTH_5_HS
#define EP_INTERVAL_5_FS	0x01  // BSB 20131022 Must set to 1 for ms
#define EP_INTERVAL_5_HS    0x04
#define EP_REFRESH_5_FS		0x05 // was: 0x05	 //  64ms, BSB 20130603: 0x05 measured as 32ms, 0x02 measured as 2ms but not always occuring
#define EP_REFRESH_5_HS		0x05 // was: 0x05	 // 2^(10-1) = 512 uframe = 64ms



// AC interface descriptor Audio specific
#define AUDIO_CLASS_REVISION			0x0100
// BSB 20130604 disabling UAC1 IN #define NB_OF_STREAMING_INTERFACE		0x02	// one for speaker out, one for mic in
#define NB_OF_STREAMING_INTERFACE		0x01	// one for speaker out, one for mic in
#define BELONGS_AUDIO_INTERFACE_OUT		STD_AS_INTERFACE_OUT
// BSB 20130604 disabling UAC1 IN #define BELONGS_AUDIO_INTERFACE_IN		STD_AS_INTERFACE_IN

// BSB 20130604 disabling UAC1 IN
/*// Input Terminal descriptor
#define INPUT_TERMINAL_ID				0x01
#define INPUT_TERMINAL_TYPE				0x0201 	// Terminal is microphone
#define INPUT_TERMINAL_ASSOCIATION		0x00   	// No association
#define INPUT_TERMINAL_NB_CHANNELS		0x02   	// Two channels for input terminal
#define INPUT_TERMINAL_CHANNEL_CONF		0x0003 	// Two channels at front left and front right positions
#define INPUT_TERMINAL_CH_NAME_ID		0x00	// No channel name

//MIC Feature Unit descriptor
#define MIC_FEATURE_UNIT_ID            0x02
#define MIC_FEATURE_UNIT_SOURCE_ID     0x01
#define MIC_FEATURE_UNIT_CONTROL_SIZE  0x02		// 2 bytes for each control
#define MIC_BMA_CONTROLS_0             0x0001 	// Mute for master channel
#define MIC_BMA_CONTROLS_1			   0x0002	// Volume control on left channel
#define MIC_BMA_CONTROLS_2			   0x0002	// Volume control on right channel
#define MIC_FEATURE_UNIT_CH_NAME_ID		0x00
*/

// Output Terminal descriptor
#define OUTPUT_TERMINAL_ID				0x03
#define OUTPUT_TERMINAL_TYPE			0x0101 	// USB Streaming
#define OUTPUT_TERMINAL_ASSOCIATION		0x00   	// No association
#define OUTPUT_TERMINAL_SOURCE_ID		0x02	// From Feature Unit Terminal

// Speaker Input Terminal
#define SPK_INPUT_TERMINAL_ID			0x11
#define SPK_INPUT_TERMINAL_TYPE			0x0101	// USB Streaming
#define SPK_INPUT_TERMINAL_ASSOCIATION	0x00	// No association
#define SPK_INPUT_TERMINAL_NB_CHANNELS	0x02	// Two channels - stereo
#define SPK_INPUT_TERMINAL_CHANNEL_CONF	0x0003	// left front and right front
#define SPK_INPUT_TERMINAL_CH_NAME_ID	0x00	// No channel name

// SPK Feature Unit
#define SPK_FEATURE_UNIT_ID				0x12
#define SPK_FEATURE_UNIT_SOURCE_ID		0x11
#define SPK_FEATURE_UNIT_CONTROL_SIZE	0x02	// 2 bytes for each control
#define SPK_FEATURE_UNIT_BMA_CONTROLS_0 0x0001	// Mute
#define SPK_FEATURE_UNIT_BMA_CONTROLS_1	0x0002	// Volume control on left front
#define SPK_FEATURE_UNIT_BMA_CONTROLS_2	0x0002	// Volume control on right front
#define SPK_FEATURE_UNIT_CH_NAME_ID		0x00	// No name

// SPK Output Terminal
#define SPK_OUTPUT_TERMINAL_ID			0x13
#define SPK_OUTPUT_TERMINAL_TYPE		0x0602	// 0x0302 = Headphones, 0x0602 = "Digital Audio Interface" or AUDIO_TE_TYPE_EXTERNAL_DIGITAL_AUDIO_INTERFACE
#define SPK_OUTPUT_TERMINAL_ASSOCIATION	0x00	// No association
#define SPK_OUTPUT_TERMINAL_SOURCE_ID	0x12	// From Feature Unit

#define SPK_ALT1_AS_NB_ENDPOINT			0x02	// OUT EP and FB EP
//#define SPK_ALT1_AS_NB_ENDPOINT			0x01	// OUT EP

#define SPK_AS_TERMINAL_LINK			0x11	// Unit Id of the speaker input terminal
#define SPK_AS_DELAY					0x04	// Interface delay
#define SPK_AS_FORMAT_TAG			    0x0001	// PCM

//Alternate O Audio Streaming (AS) interface descriptor
#define ALT0_AS_INTERFACE_INDEX			0x00   // Index of Std AS interface Alt0
#define ALT0_AS_NB_ENDPOINT				0x00   // Nb of endpoints for alt0 interface
#define ALT0_AS_INTERFACE_CLASS			0x01   // Audio class
#define ALT0_AS_INTERFACE_SUB_CLASS 	0x02   // Audio streamn sub class
#define ALT0_AS_INTERFACE_PROTOCOL		0x00   // Unused

//Alternate 1 Audio Streaming (AS) interface descriptor
#define ALT1_AS_INTERFACE_INDEX			0x01   // Index of Std AS interface Alt1
#define ALT1_AS_NB_ENDPOINT				0x01   // Nb of endpoints for alt1 interface
#define ALT1_AS_INTERFACE_CLASS			0x01   // Audio class
#define ALT1_AS_INTERFACE_SUB_CLASS 	0x02   // Audio streamn sub class
#define ALT1_AS_INTERFACE_PROTOCOL		0x00   // Unused

//AS general Interface descriptor
#define AS_TERMINAL_LINK					0x03   // Unit Id of the output terminal
#define AS_DELAY							0x01   // Interface delay
#define AS_FORMAT_TAG						0x0001 // PCM Format
// Format type for ALT1
#define FORMAT_TYPE							0x01	// Format TypeI
#define FORMAT_NB_CHANNELS					0x02	// Two Channels
#define FORMAT_FRAME_SIZE					0x03	// 3 bytes per audio sample
#define FORMAT_BIT_RESOLUTION				0x18	// 24 bits per sample
#define FORMAT_SAMPLE_FREQ_NB				0x02	// Two frequency supported
#define FORMAT_LSBYTE_SAMPLE_FREQ_441		0xac44	// 44.1khz
#define FORMAT_LSBYTE_SAMPLE_FREQ_48		0xbb80	// 48khz
#define FORMAT_MSBYTE_SAMPLE_FREQ			0x00	// MsByte
//#define FORMAT_LSBYTE_SAMPLE_FREQ			0x7700	// 96khz
//#define FORMAT_MSBYTE_SAMPLE_FREQ			0x01	// MsByte


//Audio endpoint specific descriptor field
#define AUDIO_EP_ATRIBUTES				0x01	 	// sampling freq, no pitch, no pading
#define AUDIO_EP_DELAY_UNIT				0x00	 	// Unused
#define AUDIO_EP_LOCK_DELAY				0x0000		// Unused

typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	  S_usb_configuration_descriptor cfg;
	  S_usb_interface_descriptor	 ifc0;
	  S_usb_interface_descriptor	ifc1;
	  S_usb_hid_descriptor           hid;
	  S_usb_endpoint_descriptor      ep1;
//	  S_usb_endpoint_descriptor		 ep2;
	  S_usb_interface_association_descriptor iad1;
	  S_usb_interface_descriptor     	ifc2;
	  S_usb_ac_interface_descriptor_1  	audioac;
	  // BSB 20130604 disabling UAC1 IN	  S_usb_in_ter_descriptor_1		 	mic_in_ter;
	  // BSB 20130604 disabling UAC1 IN   S_usb_feature_unit_descriptor_1  	mic_fea_unit;
	  // BSB 20130604 disabling UAC1 IN   S_usb_out_ter_descriptor_1	 	mic_out_ter;
	  S_usb_in_ter_descriptor_1			spk_in_ter;
	  S_usb_feature_unit_descriptor_1	spk_fea_unit;
	  S_usb_out_ter_descriptor_1		spk_out_ter;
	  S_usb_as_interface_descriptor 	spk_as_alt0;
	  S_usb_as_interface_descriptor 	spk_as_alt1;
	  S_usb_as_g_interface_descriptor_1	spk_g_as;
	  S_usb_format_type_1				spk_format_type;
	  S_usb_endpoint_audio_descriptor_1	ep3;
	  S_usb_endpoint_audio_specific_1 	ep3_s;
	  S_usb_endpoint_audio_descriptor_1	ep5;
	  // BSB 20130604 disabling UAC1 IN   S_usb_as_interface_descriptor 	mic_as_alt0;
	  // BSB 20130604 disabling UAC1 IN   S_usb_as_interface_descriptor 	mic_as_alt1;
	  // BSB 20130604 disabling UAC1 IN   S_usb_as_g_interface_descriptor_1	mic_g_as;
	  // BSB 20130604 disabling UAC1 IN   S_usb_format_type_1				mic_format_type;
	  // BSB 20130604 disabling UAC1 IN   S_usb_endpoint_audio_descriptor_1	ep4;
	  // BSB 20130604 disabling UAC1 IN   S_usb_endpoint_audio_specific_1 	ep4_s;
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_user_configuration_descriptor;

extern const S_usb_device_descriptor uac1_audio_usb_dev_desc;
extern const S_usb_device_descriptor uac1_dg8saq_usb_dev_desc;

#if (USB_HIGH_SPEED_SUPPORT==ENABLED)
	extern const S_usb_device_qualifier_descriptor uac1_usb_qualifier_desc;
#endif

#ifdef VDD_SENSE
extern S_usb_user_configuration_descriptor uac1_usb_conf_desc_fs;
extern S_usb_user_configuration_descriptor uac1_usb_conf_desc_fs_widget;
#else
extern const S_usb_user_configuration_descriptor uac1_usb_conf_desc_fs;
extern const S_usb_user_configuration_descriptor uac1_usb_conf_desc_fs_widget;
#endif

#if (USB_HIGH_SPEED_SUPPORT==ENABLED)
#ifdef VDD_SENSE
	extern S_usb_user_configuration_descriptor uac1_usb_conf_desc_hs;
	extern S_usb_user_configuration_descriptor uac1_usb_conf_desc_hs_widget;
#else
	extern const S_usb_user_configuration_descriptor uac1_usb_conf_desc_hs;
	extern const S_usb_user_configuration_descriptor uac1_usb_conf_desc_hs_widget;
#endif
	extern const S_usb_device_qualifier_descriptor uac1_usb_qualifier_desc;
#endif

#endif  // _UAC1_USB_DESCRIPTORS_H_
