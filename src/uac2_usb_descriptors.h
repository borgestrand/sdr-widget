/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
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
 */

#ifndef _UAC2_USB_DESCRIPTORS_H_
#define _UAC2_USB_DESCRIPTORS_H_


//_____ I N C L U D E S ____________________________________________________

#include "conf_usb.h"
#include "usb_descriptors.h"

#if USB_DEVICE_FEATURE == DISABLED
  #error uac2_usb_descriptors.h is #included although USB_DEVICE_FEATURE is disabled
#endif


#include "usb_standard_request.h"
#include "usb_task.h"
#include "hid.h" // Added BSB 20120719

//_____ U S B    D E F I N E S _____________________________________________


// CONFIGURATION
//#define NB_INTERFACE	   4	//!  DG8SAQ, Audio (3)
#define NB_INTERFACE	   5	//!  DG8SAQ, Audio (3), HID // Changed BSB 20120719
#define CONF_NB            1     //! Number of this configuration
#define CONF_INDEX         0
#define CONF_ATTRIBUTES    USB_CONFIG_BUSPOWERED	//USB_CONFIG_SELFPOWERED
#define MAX_POWER          250    // 500 mA

// IAD for Audio
#define FIRST_INTERFACE1	1
#define INTERFACE_COUNT1	3						//!  Audio Control, Audio In, Audio Out
#define FUNCTION_CLASS		AUDIO_CLASS
#define FUNCTION_SUB_CLASS  0
#define FUNCTION_PROTOCOL	IP_VERSION_02_00
#define FUNCTION_INDEX		0


// USB DG8SAQ Interface descriptor
#define INTERFACE_NB0			    0
#define ALTERNATE_NB0	            0                  //! The alt setting nb of this interface
#define NB_ENDPOINT0			    0                  //! The number of endpoints this interface has
#define INTERFACE_CLASS0		    NO_CLASS          //! No Class
#define INTERFACE_SUB_CLASS0        NO_SUBCLASS        //! No Subclass
#define INTERFACE_PROTOCOL0    		NO_PROTOCOL		   //! No Protocol
#define INTERFACE_INDEX0       		0

#define DSC_INTERFACE_DG8SAQ		INTERFACE_NB0

// BSB 20120719 HID insertion begin
// NB1 -> NB4, NB2 -> NB5

// USB HID Interface descriptor
#define INTERFACE_NB4			    4
#define ALTERNATE_NB4	            0                  //! The alt setting nb of this interface
#define NB_ENDPOINT4			    2                  //! The number of endpoints this interface has
#define INTERFACE_CLASS4		    HID_CLASS          //! HID Class
#define INTERFACE_SUB_CLASS4        NO_SUBCLASS        //! No Subclass
#define INTERFACE_PROTOCOL4    		NO_PROTOCOL		   //! No Protocol
#define INTERFACE_INDEX4       		0

#define DSC_INTERFACE_HID			INTERFACE_NB4

// HID descriptor
#define HID_VERSION                 0x0111  //! HID Class Specification release number
#define HID_COUNTRY_CODE            0x00    //! Hardware target country
#define HID_NUM_DESCRIPTORS			0x01    //! Number of HID class descriptors to follow

// USB Endpoint 4 descriptor
#define ENDPOINT_NB_4           (UAC2_EP_HID_TX| MSK_EP_DIR)
#define EP_ATTRIBUTES_4         TYPE_INTERRUPT
#define EP_IN_LENGTH_4_FS       8
#define EP_SIZE_4_FS            EP_IN_LENGTH_4_FS
#define EP_IN_LENGTH_4_HS       8
#define EP_SIZE_4_HS            EP_IN_LENGTH_4_HS
#define EP_INTERVAL_4           5               //! Interrupt polling interval from host

// USB Endpoint 5 descriptor
#define ENDPOINT_NB_5           (UAC2_EP_HID_RX)
#define EP_ATTRIBUTES_5         TYPE_INTERRUPT
#define EP_OUT_LENGTH_5_FS      8
#define EP_SIZE_5_FS            EP_OUT_LENGTH_5_FS
#define EP_OUT_LENGTH_5_HS      8
#define EP_SIZE_5_HS            EP_OUT_LENGTH_5_HS
#define EP_INTERVAL_5           5               //! Interrupt polling interval from host

// BSB 20120719 HID insertion end


// Audio Class V2.0 descriptor values

// Standard Audio Control (AC) interface descriptor
#define INTERFACE_NB1       1
#define ALTERNATE_NB1       0
#define NB_ENDPOINT1        0			     //! No endpoint for AC interface
#define INTERFACE_CLASS1    AUDIO_CLASS  	//! Audio Class
#define INTERFACE_SUB_CLASS1 AUDIO_INTERFACE_SUBCLASS_AUDIOCONTROL
#define INTERFACE_PROTOCOL1  IP_VERSION_02_00		     //! IP_VERSION_02_00 ie UAC V2
#define INTERFACE_INDEX1     WL_INDEX

#define DSC_INTERFACE_AUDIO			INTERFACE_NB1


// USB Endpoint 1 descriptor
#define ENDPOINT_NB_1       ( UAC2_EP_AUDIO_IN | MSK_EP_DIR )
#define EP_ATTRIBUTES_1		0b00100101         // ISOCHROUNOUS ASYNCHRONOUS IMPLICIT FEEDBACK
//#define EP_IN_LENGTH_1_FS	294				// 3 bytes * 49 samples * stereo
//#define EP_IN_LENGTH_1_HS	294
#define EP_IN_LENGTH_1_FS	392				// 4 bytes * 49 samples * stereo
#define EP_IN_LENGTH_1_HS	392
#define EP_SIZE_1_FS		EP_IN_LENGTH_1_FS
#define EP_SIZE_1_HS        EP_IN_LENGTH_1_HS
#define EP_INTERVAL_1_FS	0x01			 // one packet per uframe, each uF 1ms, so only 48khz
#define EP_INTERVAL_1_HS    0x02			 // One packet per 2 uframe, each uF 125us, so 192khz


// USB Endpoint 2 descriptor
#define ENDPOINT_NB_2       ( UAC2_EP_AUDIO_OUT )
#define EP_ATTRIBUTES_2     0b00000101			// ISOCHRONOUS ASYNC
//#define EP_OUT_LENGTH_2_HS  294				// 3 bytes * 49 samples * stereo
//#define EP_OUT_LENGTH_2_FS	294
#define EP_OUT_LENGTH_2_HS  392				// 4 bytes * 49 samples * stereo
#define EP_OUT_LENGTH_2_FS	392
#define EP_SIZE_2_FS		EP_OUT_LENGTH_2_FS
#define EP_SIZE_2_HS        EP_OUT_LENGTH_2_HS
#define EP_INTERVAL_2_FS	0x01			 // one packet per uframe
#define EP_INTERVAL_2_HS    0x02			 // One packet per 2 uframe


// USB Endpoint 3 descriptor
#define ENDPOINT_NB_3       ( UAC2_EP_AUDIO_OUT_FB | MSK_EP_DIR )		// 0x83
#define EP_ATTRIBUTES_3     0b00010001      // ISOCHRONOUS FEEDBACK
#define EP_IN_LENGTH_3_FS   4 //64				//
#define EP_IN_LENGTH_3_HS	4 //64				//
#define EP_SIZE_3_FS		EP_IN_LENGTH_3_FS
#define EP_SIZE_3_HS        EP_IN_LENGTH_3_HS
#define EP_INTERVAL_3_FS	0x01
#define EP_INTERVAL_3_HS    0x04
#define EP_REFRESH_3_FS		0x05			 //  16ms
#define EP_REFRESH_3_HS		0x08			 // 2^(8-1) = 128 uframe = 16ms

// AC interface descriptor Audio specific
#define AUDIO_CLASS_REVISION_2          0x0200
#define MIC_CATEGORY					AUDIO_FUNCTION_SUBCLASS_MICROPHONE
#define HEADSET_CATEGORY 				0x04
#define MIC_LATENCY_CONTROL				0b00000000

// Clock Source descriptor
#define CSD_ID_1						0x04
#define CSD_ID_1_TYPE					0b00000001	// fixed freq internal clock
#define CSD_ID_1_CONTROL				0b00000111	// freq r/w, validity r
#define CSD_ID_2						0x05
#define CSD_ID_2_TYPE					0b00000001	// fixed freq internal clock
#define CSD_ID_2_CONTROL				0b00000111	// freq r/w, validity r

// Clock Selector descriptor
#define CSX_ID							0x06
#define CSX_INPUT_PINS					0x02
#define CSX_SOURCE_1					CSD_ID_1
#define CSX_SOURCE_2					CSD_ID_2
#define CSX_CONTROL						0b00000011	// clock selector is readable and writable


// Input Terminal descriptor
#define INPUT_TERMINAL_ID				0x01
#define INPUT_TERMINAL_TYPE				0x0201 	// Terminal is microphone
#define INPUT_TERMINAL_ASSOCIATION		0x00   	// No association
#define INPUT_TERMINAL_NB_CHANNELS		0x02   	// Two channels for input terminal
#define INPUT_TERMINAL_CHANNEL_CONF		0x00000003 	// Two channels at front left and front right positions
//#define INPUT_TERMINAL_CONTROLS			0x0040	// D7-6 Cluster control - readonly
#define INPUT_TERMINAL_CONTROLS			0x0000	// none
#define INPUT_TERMINAL_CH_NAME_ID		0x00	// No channel name
#define INPUT_TERMINAL_STRING_DESC	    0x00	// No string descriptor

// Output Terminal descriptor
#define OUTPUT_TERMINAL_ID				0x03
#define OUTPUT_TERMINAL_TYPE			0x0101 	// USB Streaming
#define OUTPUT_TERMINAL_ASSOCIATION		0x00   	// No association
#define OUTPUT_TERMINAL_SOURCE_ID		MIC_FEATURE_UNIT_ID
#define OUTPUT_TERMINAL_CONTROLS		0x0000	// no controls


//MIC Feature Unit descriptor
#define MIC_FEATURE_UNIT_ID            0x02
#define MIC_FEATURE_UNIT_SOURCE_ID     INPUT_TERMINAL_ID
#define MIC_BMA_CONTROLS               0x00000003 	// Mute readable and writable
#define MIC_BMA_CONTROLS_CH_1		   0x00000003	//
#define MIC_BMA_CONTROLS_CH_2		   0x00000003

// Speaker Input Terminal
#define SPK_INPUT_TERMINAL_ID			0x11
#define SPK_INPUT_TERMINAL_TYPE			0x0101	// USB Streaming
#define SPK_INPUT_TERMINAL_ASSOCIATION	0x00	// No association
#define SPK_INPUT_TERMINAL_NB_CHANNELS	0x02
#define SPK_INPUT_TERMINAL_CHANNEL_CONF	0x0003	// left front and right front
#define SPK_INPUT_TERMINAL_CH_NAME_ID	0x00	
#define SPK_INPUT_TERMINAL_STRING_DESC	AIT_INDEX

//SPK Feature Unit descriptor
#define SPK_FEATURE_UNIT_ID            0x12
#define SPK_FEATURE_UNIT_SOURCE_ID     SPK_INPUT_TERMINAL_ID
#define SPK_BMA_CONTROLS               0x00000003 	// Mute readable and writable
#define SPK_BMA_CONTROLS_CH_1		   0x00000003	//
#define SPK_BMA_CONTROLS_CH_2		   0x00000003

// SPK Output Terminal descriptor
#define SPK_OUTPUT_TERMINAL_ID				0x13
#define SPK_OUTPUT_TERMINAL_TYPE		0x0302	// Headphones
#define SPK_OUTPUT_TERMINAL_ASSOCIATION		0x00   	// No association
#define SPK_OUTPUT_TERMINAL_SOURCE_ID		SPK_FEATURE_UNIT_ID
#define SPK_OUTPUT_TERMINAL_CONTROLS		0x0000	// no controls

//Audio Streaming (AS) interface descriptor
#define STD_AS_INTERFACE_IN				0x03   // Index of Std AS Interface for Audio In
#define STD_AS_INTERFACE_OUT			0x02   // Index of Std AS Interface for Audio Out

#define DSC_INTERFACE_AS				STD_AS_INTERFACE_IN
#define DSC_INTERFACE_AS_OUT			STD_AS_INTERFACE_OUT


//Alternate O Audio Streaming (AS) interface descriptor
#define ALT0_AS_INTERFACE_INDEX			0x00   // Index of Std AS interface Alt0
#define ALT0_AS_NB_ENDPOINT				0x00   // Nb of endpoints for alt0 interface
#define ALT0_AS_INTERFACE_CLASS			0x01   // Audio class
#define ALT0_AS_INTERFACE_SUB_CLASS 	0x02   // Audio streamn sub class
#define ALT0_AS_INTERFACE_PROTOCOL		IP_VERSION_02_00


//Alternate 1 Audio Streaming (AS) interface descriptor
#define ALT1_AS_INTERFACE_INDEX			0x01   // Index of Std AS interface Alt1
#define ALT1_AS_NB_ENDPOINT				0x01   // Nb of endpoints for alt1 interface
#define ALT1_AS_INTERFACE_CLASS			0x01   // Audio class
#define ALT1_AS_INTERFACE_SUB_CLASS 	0x02   // Audio streamn sub class
#define ALT1_AS_INTERFACE_PROTOCOL		IP_VERSION_02_00


//Alternate 2 Audio Streaming (AS) interface descriptor
#define ALT2_AS_INTERFACE_INDEX			0x02   // Index of Std AS interface Alt2
#define ALT2_AS_NB_ENDPOINT				0x01   // Nb of endpoints for alt2 interface
#define ALT2_AS_INTERFACE_CLASS			0x01   // Audio class
#define ALT2_AS_INTERFACE_SUB_CLASS 	0x02   // Audio streamn sub class
#define ALT2_AS_INTERFACE_PROTOCOL		IP_VERSION_02_00


//Class Specific AS (general) Interface descriptor
#define AS_TERMINAL_LINK					OUTPUT_TERMINAL_ID		// Unit Id of the output terminal
#define AS_DELAY							0x01		// Interface delay
#define AS_FORMAT_TYPE						0x01		// PCM Format
#define AS_FORMATS							0x00000001	// PCM only
#define AS_CONTROLS							0b00000111	// active alt settings r/w, valid alt settings r
#define AS_NB_CHANNELS						0x02
#define AS_CHAN_CONFIG						0x00000003	// L+R front

// Format type for ALT1
#define FORMAT_TYPE_1						0x01	// Format TypeI
#define FORMAT_SUBSLOT_SIZE_1				0x04	// Number of bytes per subslot
#define FORMAT_BIT_RESOLUTION_1				0x18	// 24 bits per sample

//Audio endpoint specific descriptor field
#define AUDIO_EP_ATRIBUTES				0b00000000	 	// No sampling freq, no pitch, no pading
#define AUDIO_EP_CONTROLS				0b00000000
#define AUDIO_EP_DELAY_UNIT				0x00	 	// Unused
#define AUDIO_EP_LOCK_DELAY				0x0000		// Unused

//For playback
#define ALT1_AS_NB_ENDPOINT_OUT			0x02   // two EP,  OUT and OUT_FB
#define AS_TERMINAL_LINK_OUT		    SPK_INPUT_TERMINAL_ID

//! Usb Class-Specific AS Isochronous Feedback Endpoint Descriptors pp 4.10.2.2 (none)

typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	S_usb_configuration_descriptor			cfg;
	S_usb_interface_descriptor	 			ifc0;

	//! Audio descriptors Class 2

	S_usb_interface_association_descriptor	iad1;
	S_usb_interface_descriptor				ifc1;
	S_usb_ac_interface_descriptor_2			audioac;
//	S_usb_clock_source_descriptor			audio_cs1;
	S_usb_clock_source_descriptor			audio_cs2;
//	S_usb_clock_selector_descriptor			audio_csel;
	//			S_usb_clock_multiplier_descriptor		audio_cmul;
/*
	S_usb_in_ter_descriptor_2 				mic_in_ter;
	S_usb_feature_unit_descriptor_2			mic_fea_unit;
	S_usb_out_ter_descriptor_2				mic_out_ter;
*/
	S_usb_in_ter_descriptor_2				spk_in_ter;
	S_usb_feature_unit_descriptor_2			spk_fea_unit;
	S_usb_out_ter_descriptor_2				spk_out_ter;
	S_usb_as_interface_descriptor	 		spk_as_alt0;
	S_usb_as_interface_descriptor	 		spk_as_alt1;
	S_usb_as_g_interface_descriptor_2		spk_g_as;
	S_usb_format_type_2						spk_format_type;
	S_usb_endpoint_audio_descriptor_2 		ep2;
	S_usb_endpoint_audio_specific_2			ep2_s;
	S_usb_endpoint_audio_descriptor_2 		ep3;

	// BSB 20120720 Added
	S_usb_interface_descriptor		ifc4;
	S_usb_hid_descriptor           	hid;
	S_usb_endpoint_descriptor      	ep4;
	S_usb_endpoint_descriptor	   	ep5;


/*
	S_usb_as_interface_descriptor	 		mic_as_alt0;
	S_usb_as_interface_descriptor	 		mic_as_alt1;
	S_usb_as_g_interface_descriptor_2		mic_g_as;
	S_usb_format_type_2						mic_format_type;
	S_usb_endpoint_audio_descriptor_2 		ep1;
	S_usb_endpoint_audio_specific_2			ep1_s;
*/
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_user_configuration_descriptor;

extern const S_usb_device_descriptor uac2_dg8saq_usb_dev_desc;
extern const S_usb_device_descriptor uac2_audio_usb_dev_desc;
extern const S_usb_user_configuration_descriptor uac2_usb_conf_desc_fs;
#if USB_HIGH_SPEED_SUPPORT==ENABLED
extern const S_usb_user_configuration_descriptor uac2_usb_conf_desc_hs;
extern const S_usb_device_qualifier_descriptor uac2_usb_qualifier_desc;
#endif
#endif  // _UAC2_USB_DESCRIPTORS_H_
