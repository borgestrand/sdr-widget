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

// FEATURE_ADC_EXPERIMENTAL Add one more for Audio IN?

#ifdef FEATURE_ADC_EXPERIMENTAL
	#ifdef FEATURE_HID
		#define NB_INTERFACE	4  //         Audio control, audio streaming, audio recording, HID
	#else // no HID
		#define NB_INTERFACE	3  //         Audio control, audio streaming, audio recording
	#endif
#else // no ADC
	#ifdef FEATURE_HID
		#define NB_INTERFACE	3  //         Audio control, audio streaming, HID
	#else // no HID
		#define NB_INTERFACE	2  //         Audio control, audio streaming
	#endif
#endif

#define CONF_NB            				1	//! Number of this configuration
#define CONF_INDEX         				0
#define CONF_ATTRIBUTES    				USB_CONFIG_BUSPOWERED  // default is bus-powered
#define MAX_POWER          				250 // 500mA

// IAD for Audio
#define FIRST_INTERFACE1				0	// No config interface, bFirstInterface = 0
#ifdef FEATURE_ADC_EXPERIMENTAL
	#define INTERFACE_COUNT1				3						//!  Audio Control, Audio In, Audio Out
#else
	#define INTERFACE_COUNT1				2						//!  Audio Control, Audio Out
#endif
#define FUNCTION_CLASS					AUDIO_CLASS
#define FUNCTION_SUB_CLASS  			0
#define FUNCTION_PROTOCOL				IP_VERSION_02_00
#define FUNCTION_INDEX					0


// BSB 20120719 HID insertion begin
// In most cases: translation from uac1 code follows pattern of NB1 -> NB4, NB2 -> NB5

// USB HID Interface descriptor, this is the last USB interface!
#ifdef FEATURE_HID

#ifdef FEATURE_ADC_EXPERIMENTAL
	#define INTERFACE_NB3			    3	// No config interface, ADC interface, HID interface = 3
#else
	#define INTERFACE_NB3			    2	// No config interface, HID interface = 2
#endif

// ADC_site is this setting ADC dependent??


	#define ALTERNATE_NB3	            	0                  //! The alt setting nb of this interface
	#define NB_ENDPOINT3			    	1 // 2             //! The number of endpoints this interface has
	#define INTERFACE_CLASS3		    	HID_CLASS          //! HID Class
	#define INTERFACE_SUB_CLASS3        	NO_SUBCLASS        //! No Subclass
	#define INTERFACE_PROTOCOL3    			NO_PROTOCOL		   //! No Protocol
	#define INTERFACE_INDEX3       			0

	#define DSC_INTERFACE_HID				INTERFACE_NB3

	// HID descriptor
	#define HID_VERSION                 	0x0111  //! HID Class Specification release number
	#define HID_COUNTRY_CODE            	0x00    //! Hardware target country
	#define HID_NUM_DESCRIPTORS				0x01    //! Number of HID class descriptors to follow

	// USB Endpoint 4 descriptor for HID TX
	#define ENDPOINT_NB_4           		(UAC2_EP_HID_TX| MSK_EP_DIR)
	#define EP_ATTRIBUTES_4         		TYPE_INTERRUPT
	#define EP_IN_LENGTH_4_FS       		8
	#define EP_SIZE_4_FS            		EP_IN_LENGTH_4_FS
	#define EP_IN_LENGTH_4_HS       		8
	#define EP_SIZE_4_HS            		EP_IN_LENGTH_4_HS
	#define EP_INTERVAL_4_FS           		16 // frames = 16ms was: 5    //! Interrupt polling interval from host
	#define EP_INTERVAL_4_HS        		16 // microframes = 2ms, here: 4ms was: 0x05    //! Interrupt polling interval from host

	/*
	// USB Endpoint 5 descriptor for HID RX - not used
	#define ENDPOINT_NB_5           		(UAC2_EP_HID_RX)
	#define EP_ATTRIBUTES_5         		TYPE_INTERRUPT
	#define EP_OUT_LENGTH_5_FS      		8
	#define EP_SIZE_5_FS            		EP_OUT_LENGTH_5_FS
	#define EP_OUT_LENGTH_5_HS      		8
	#define EP_SIZE_5_HS            		EP_OUT_LENGTH_5_HS
	#define EP_INTERVAL_5           		5               //! Interrupt polling interval from host
	*/
#endif
// BSB 20120719 HID insertion end


// Audio Class V2.0 descriptor values

// Standard Audio Control (AC) interface descriptor
#define INTERFACE_NB1       			0				// No config interface, Audio control interface = 0
#define ALTERNATE_NB1       			0
#define NB_ENDPOINT1        			0			     //! No endpoint for AC interface
#define INTERFACE_CLASS1    			AUDIO_CLASS  	 //! Audio Class
#define INTERFACE_SUB_CLASS1 			AUDIO_INTERFACE_SUBCLASS_AUDIOCONTROL
#define INTERFACE_PROTOCOL1  			IP_VERSION_02_00		     //! IP_VERSION_02_00 ie UAC V2
#define INTERFACE_INDEX1     			WL_INDEX

#define DSC_INTERFACE_AUDIO				INTERFACE_NB1


// USB Endpoint 1 descriptor - audio in - not used for pure USB DACs
#define ENDPOINT_NB_1       			( UAC2_EP_AUDIO_IN | MSK_EP_DIR ) // 0x83
#define EP_ATTRIBUTES_1					0b00100101         // ISOCHROUNOUS ASYNCHRONOUS IMPLICIT FEEDBACK
#define EP_IN_LENGTH_1_FS				300				   // 3 bytes * 49+1 samples * stereo
#define EP_IN_LENGTH_1_HS				300				   // Matches FORMAT_SUBSLOT_SIZE_1	0x03
//#define EP_IN_LENGTH_1_FS				392				   // 4 bytes * 49 samples * stereo
//#define EP_IN_LENGTH_1_HS				392				   // Matches FORMAT_SUBSLOT_SIZE_1	0x04
#define EP_SIZE_1_FS					EP_IN_LENGTH_1_FS
#define EP_SIZE_1_HS        			EP_IN_LENGTH_1_HS
#define EP_INTERVAL_1_FS				0x01			   // one packet per uframe, each uF 1ms, so only 48khz
#define EP_INTERVAL_1_HS    			0x02			   // One packet per 2 uframe, each uF 125us, so 192khz

// USB Endpoint 2 descriptor (for EP numbering see conf_usb.h)
#define ENDPOINT_NB_2       			( UAC2_EP_AUDIO_OUT )	// 0x02
#define EP_ATTRIBUTES_2     			0b00000101			// ISOCHRONOUS ASYNC
#define EP_OUT_LENGTH_2_FS				300				   // 3 bytes * 49+1 samples * stereo
#define EP_OUT_LENGTH_2_HS				300				   // Matches FORMAT_SUBSLOT_SIZE_1	0x03, it is larger than FORMAT_SUBSLOT_SIZE_2 anyway
//#define EP_OUT_LENGTH_2_FS			392				   // 4 bytes * 49 samples * stereo
//#define EP_OUT_LENGTH_2_HS			392				   // Matches FORMAT_SUBSLOT_SIZE_1	0x04
#define EP_SIZE_2_FS					EP_OUT_LENGTH_2_FS
#define EP_SIZE_2_HS        			EP_OUT_LENGTH_2_HS
#define EP_INTERVAL_2_FS				0x01			 // one packet per frame
#define EP_INTERVAL_2_HS    			0x02			 // One packet per 2 uframe

// USB Endpoint 3 descriptor
#define ENDPOINT_NB_3       			( UAC2_EP_AUDIO_OUT_FB | MSK_EP_DIR )		// 0x81
#define EP_ATTRIBUTES_3     			0b00010001      // ISOCHRONOUS FEEDBACK
#define EP_IN_LENGTH_3_FS   			4 //64				//   BSB 20170703 Is this really correct? Never tested in FS
#define EP_IN_LENGTH_3_HS				4 //64				// Xperia, changing to 64 did not help
#define EP_SIZE_3_FS					EP_IN_LENGTH_3_FS
#define EP_SIZE_3_HS        			EP_IN_LENGTH_3_HS
#define EP_INTERVAL_3_FS				0x01
#define EP_INTERVAL_3_HS   				0x04 // High number is good! /<= 4 works on Windows. Anything works on Linux. 4 OK on Mac

// AC interface descriptor Audio specific
#define AUDIO_CLASS_REVISION_2          0x0200
#define MIC_CATEGORY					AUDIO_FUNCTION_SUBCLASS_MICROPHONE
#define HEADSET_CATEGORY 				AUDIO_FUNCTION_SUBCLASS_IO_BOX // Was: AUDIO_FUNCTION_SUBCLASS_HEADSET // Was hard-coded 0x04
#define MIC_LATENCY_CONTROL				0b00000000

// Clock Source descriptor - CSD_ID_1 not used
#define CSD_ID_1						0x04
#define CSD_ID_1_TYPE					0b00000010	// Was: 01 fixed freq internal clock. Is: 10 var. int.
#define CSD_ID_1_CONTROL				0b00000111	// freq r/w, validity r
#define CSD_ID_2						0x05
#define CSD_ID_2_TYPE					0b00000011	// Was: 01 fixed freq internal clock. Or: 10 var. int. Is: 11 programmable
#define CSD_ID_2_CONTROL				0b00000111	// Was: 00000111 freq r/w, validity r


// Clock Selector descriptor - not used
#ifdef FEATURE_CLOCK_SELECTOR				// Only if clock selector is compiled in do we expose it in the feature unit
	#define CSX_ID							0x06
	#define CSX_INPUT_PINS					0x01		// This must match the single clock source being used!
	#define CSX_SOURCE_1					CSD_ID_1
	//#define CSX_SOURCE_2					CSD_ID_2	// Only a single clock source going into clock selector
	#define CSX_CONTROL						0b00000011	// clock selector is readable and writable
#endif

// Input Terminal descriptor - for ADC_site support
#define INPUT_TERMINAL_ID				0x01
#define INPUT_TERMINAL_TYPE				0x0201 	// Terminal is microphone
#define INPUT_TERMINAL_ASSOCIATION		0x00   	// No association
#define INPUT_TERMINAL_NB_CHANNELS		0x02   	// Was: '2 // Two channels for input terminal
#define INPUT_TERMINAL_CHANNEL_CONF		0x00000003 	// Was: '3 // Two channels at front left and front right positions
#define INPUT_TERMINAL_CONTROLS			0x0000	// none Was: 0x0040	// D7-6 Cluster control - readonly
#define INPUT_TERMINAL_CH_NAME_ID		0x00	// No channel name
#define INPUT_TERMINAL_STRING_DESC	    0x00	// No string descriptor

// Output Terminal descriptor - for ADC_site support
#define OUTPUT_TERMINAL_ID				0x03
#define OUTPUT_TERMINAL_TYPE			0x0101 	// USB Streaming
#define OUTPUT_TERMINAL_ASSOCIATION		0x00   	// No association
#define OUTPUT_TERMINAL_SOURCE_ID		INPUT_TERMINAL_ID // was: MIC_FEATURE_UNIT_ID // Does INPUT_TERMINAL_ID work?
#define OUTPUT_TERMINAL_CONTROLS		0x0000	// no controls

/* mic_feature_unit removed from code here
//MIC Feature Unit descriptor - reintroducing for ADC_site. Present in master branch on github
#define MIC_FEATURE_UNIT_ID            0x02
#define MIC_FEATURE_UNIT_SOURCE_ID     INPUT_TERMINAL_ID
#define MIC_BMA_CONTROLS               0x00000003 	// Mute readable and writable
#define MIC_BMA_CONTROLS_CH_1		   0x00000003	//
#define MIC_BMA_CONTROLS_CH_2		   0x00000003
*/

// Speaker Input Terminal
#define SPK_INPUT_TERMINAL_ID			0x11
#define SPK_INPUT_TERMINAL_TYPE			0x0101	// USB Streaming
#define SPK_INPUT_TERMINAL_ASSOCIATION	0x00	// No association
#define SPK_INPUT_TERMINAL_NB_CHANNELS	0x02
#define SPK_INPUT_TERMINAL_CHANNEL_CONF	0x0000 	// 0x0000 suggested by Tsai and used elsewhere. Was: // 0x0003	// left front and right front
#define SPK_INPUT_TERMINAL_CH_NAME_ID	LEFT_CH_INDEX // Was: 0x00
#define SPK_INPUT_TERMINAL_STRING_DESC	AIT_INDEX

//SPK Feature Unit descriptor
#ifdef FEATURE_VOLUME_CTRL				// Only if volume control is compiled in do we expose it in the feature unit
	#define SPK_FEATURE_UNIT_ID         0x14	// Was 0x12
	#define SPK_FEATURE_UNIT_SOURCE_ID  SPK_INPUT_TERMINAL_ID
	#define SPK_BMA_CONTROLS           	0x00000003 	// Mute master channel. [Readable and writable ?]
	#define SPK_BMA_CONTROLS_CH_1		0x0000000C	// Volume control L
	#define SPK_BMA_CONTROLS_CH_2		0x0000000C	// Volume control R
#endif

// SPK Output Terminal descriptor
#define SPK_OUTPUT_TERMINAL_ID			0x13
#define SPK_OUTPUT_TERMINAL_TYPE		AUDIO_TE_TYPE_EXTERNAL_LINE_CONNECTOR // AUDIO_TE_TYPE_OUTPUT_SPEAKER // Speakers. Was: 0x0603 // Analog line out. Was: 0x0602	// 0x0302 for Headphones. Alternatively, 0x0602, "Digital Audio Interface" }Headphones or AUDIO_TE_TYPE_EXTERNAL_DIGITAL_AUDIO_INTERFACE
#define SPK_OUTPUT_TERMINAL_ASSOCIATION	0x00   	// No association
#ifdef FEATURE_VOLUME_CTRL				// Only if volume control is compiled in do we expose it in the feature unit
	#define SPK_OUTPUT_TERMINAL_SOURCE_ID	SPK_FEATURE_UNIT_ID
#else
	#define SPK_OUTPUT_TERMINAL_SOURCE_ID	SPK_INPUT_TERMINAL_ID
#endif
#define SPK_OUTPUT_TERMINAL_CONTROLS	0x0000	// no controls

//Audio Streaming (AS) interface descriptor

#ifdef FEATURE_ADC_EXPERIMENTAL
	#define STD_AS_INTERFACE_OUT		 0x02 // Was: 0x01 OUT comes after IN in descriptors, but before it in endponts. 0x01   // Index of Std AS Interface for Audio Out
#else
	#define STD_AS_INTERFACE_OUT		 0x01
#endif


//#define STD_AS_INTERFACE_OUT		 0x01 // Truly experimental, OUT comes after IN in descriptors. 0x01   // Index of Std AS Interface for Audio Out

#define DSC_INTERFACE_AS_OUT			STD_AS_INTERFACE_OUT

// ADC_site audio streaming in interface - highly experimental

// Bringing back ADC support from main branch
#ifdef FEATURE_ADC_EXPERIMENTAL		// ADC_site number of interfaces
	//Audio Streaming (AS) interface descriptor
	#define STD_AS_INTERFACE_IN			0x01 // Truly experimental, OUT comes after IN in descriptors. 0x02   // Index of Std AS Interface for Audio In, one more than the Audio Out one. That's a gamble!!

	#define DSC_INTERFACE_AS			STD_AS_INTERFACE_IN
#endif



//Alternate O Audio Streaming (AS) interface descriptor
#define ALT0_AS_INTERFACE_INDEX			0x00   // Index of Std AS interface Alt0
#define ALT0_AS_NB_ENDPOINT				0x00   // Nb of endpoints for alt0 interface
#define ALT0_AS_INTERFACE_CLASS			0x01   // Audio class
#define ALT0_AS_INTERFACE_SUB_CLASS 	0x02   // Audio streamn sub class
#define ALT0_AS_INTERFACE_PROTOCOL		IP_VERSION_02_00

//Alternate 1 Audio Streaming (AS) interface descriptor
#define ALT1_AS_INTERFACE_INDEX			0x01   // Index of Std AS interface Alt1
#define ALT1_AS_NB_ENDPOINT				0x01   // Nb of endpoints for alt1 interface, is this for Audio IN?
#define ALT1_AS_INTERFACE_CLASS			0x01   // Audio class
#define ALT1_AS_INTERFACE_SUB_CLASS 	0x02   // Audio streamn sub class
#define ALT1_AS_INTERFACE_PROTOCOL		IP_VERSION_02_00

//Alternate 2 Audio Streaming (AS) interface descriptor // bBitResolution
#define ALT2_AS_INTERFACE_INDEX			0x02   // Index of Std AS interface Alt2
#define ALT2_AS_NB_ENDPOINT				0x01   // Nb of endpoints for alt2 interface
#define ALT2_AS_INTERFACE_CLASS			0x01   // Audio class
#define ALT2_AS_INTERFACE_SUB_CLASS 	0x02   // Audio streamn sub class
#define ALT2_AS_INTERFACE_PROTOCOL		IP_VERSION_02_00

//Class Specific AS (general) Interface descriptor
#define AS_TERMINAL_LINK				OUTPUT_TERMINAL_ID		// Unit Id of the output terminal
#define AS_DELAY						0x01		// Interface delay
#define AS_FORMAT_TYPE					0x01		// PCM Format
#define AS_FORMATS						0x00000001	// PCM only
#define AS_CONTROLS						0x00 // Recommended for UAC2 0b00000111	// active alt settings r/w, valid alt settings r
#define AS_NB_CHANNELS					0x02
#define AS_CHAN_CONFIG					0x00000000 	// Recommended. Was: 0x00000003	// L+R front

// Format type for ALT1
#define FORMAT_TYPE_1					0x01	// Format TypeI
#define FORMAT_SUBSLOT_SIZE_1			0x03	// ADC_site // Number of bytes per subslot 20230223 why was this 4? Keeping it at 4 breaks ADC functionality on Win10
#define FORMAT_BIT_RESOLUTION_1			0x18	// 24 bits per sample

// Format type for ALT2 // bBitResolution
#define FORMAT_TYPE_2					0x01	// Format TypeI
#define FORMAT_SUBSLOT_SIZE_2			0x02	// Number of bytes per subslot
#define FORMAT_BIT_RESOLUTION_2			0x10	// 16 bits per sample

//Audio endpoint specific descriptor field
#define AUDIO_EP_ATRIBUTES				0b00000000	 	// No sampling freq, no pitch, no pading
#define AUDIO_EP_CONTROLS				0b00000000
#define AUDIO_EP_DELAY_UNIT				0x00	 	// Unused
#define AUDIO_EP_LOCK_DELAY				0x0000		// Unused

//For playback
#define ALT1_AS_NB_ENDPOINT_OUT			0x02   // two EP,  OUT and OUT_FB
#define ALT2_AS_NB_ENDPOINT_OUT			0x02   // two EP,  OUT and OUT_FB
#define AS_TERMINAL_LINK_OUT		    SPK_INPUT_TERMINAL_ID

//! Usb Class-Specific AS Isochronous Feedback Endpoint Descriptors pp 4.10.2.2 (none)

// ADC_site UAC2 descriptor
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

	//! Audio descriptors Class 2
	S_usb_interface_association_descriptor	iad1;
	S_usb_interface_descriptor				ifc1;
	S_usb_ac_interface_descriptor_2			audioac;
	S_usb_clock_source_descriptor			audio_cs2;
#ifdef FEATURE_CLOCK_SELECTOR				// Only if clock selector is compiled in do we expose it in the feature unit
	S_usb_clock_selector_descriptor			audio_csel; // ClockSelector
#endif


#ifdef FEATURE_ADC_EXPERIMENTAL		// Brought back from main branch
	S_usb_in_ter_descriptor_2 				mic_in_ter;
//	S_usb_feature_unit_descriptor_2			mic_fea_unit;	// Retain microphone gain / mute control from main branch	// implies #define OUTPUT_TERMINAL_SOURCE_ID	INPUT_TERMINAL_ID somewhere. And those IDs must be unique I guess
	S_usb_out_ter_descriptor_2				mic_out_ter;
#endif


// Speaker output terminal - not changed
	S_usb_in_ter_descriptor_2				spk_in_ter;
#ifdef FEATURE_VOLUME_CTRL				// Only if volume control is compiled in do we expose it in the feature unit
	S_usb_feature_unit_descriptor_2			spk_fea_unit;
#endif
	S_usb_out_ter_descriptor_2				spk_out_ter;


#ifdef FEATURE_ADC_EXPERIMENTAL		// Brought back from main branch
	// Mic alt0
	S_usb_as_interface_descriptor	 		mic_as_alt0;

	// Mic alt1
	S_usb_as_interface_descriptor	 		mic_as_alt1;
	S_usb_as_g_interface_descriptor_2		mic_g_as;
	S_usb_format_type_2						mic_format_type;
	S_usb_endpoint_audio_descriptor_2 		ep1;
	S_usb_endpoint_audio_specific_2			ep1_s;

	// Mic alt2 - identical for now, may change bit resolution to 16 if there is a way to test it
// ADC_site skipping alt2 for now
//	S_usb_as_interface_descriptor	 		mic_as_alt2;
//	S_usb_as_g_interface_descriptor_2		mic_g_as_alt2;
//	S_usb_format_type_2						mic_format_type_alt2;
//	S_usb_endpoint_audio_descriptor_2 		ep1_alt2;
//	S_usb_endpoint_audio_specific_2			ep1_s_alt2;
#endif


	// Speaker alt0
	S_usb_as_interface_descriptor	 		spk_as_alt0;

	// Speaker alt1
	S_usb_as_interface_descriptor	 		spk_as_alt1;
	S_usb_as_g_interface_descriptor_2		spk_g_as;
	S_usb_format_type_2						spk_format_type;
	S_usb_endpoint_audio_descriptor_2 		ep2;
	S_usb_endpoint_audio_specific_2			ep2_s;
	S_usb_endpoint_audio_descriptor_2 		ep3;

	// Speaker alt2 bBitResolution added alt2 for 16-bit audio streaming
// ADC_site skipping alt2 for now
//	S_usb_as_interface_descriptor	 		spk_as_alt2;
//	S_usb_as_g_interface_descriptor_2		spk_g_as_alt2;
//	S_usb_format_type_2						spk_format_type_alt2;
//	S_usb_endpoint_audio_descriptor_2 		ep2_alt2;
//	S_usb_endpoint_audio_specific_2			ep2_s_alt2;
//	S_usb_endpoint_audio_descriptor_2 		ep3_alt2;

	// BSB 20120720 Added, reduced to ONE TX endpoint
#ifdef FEATURE_HID
	S_usb_interface_descriptor				ifc3;
	S_usb_hid_descriptor           			hid;
	S_usb_endpoint_descriptor     		 	ep4;
#endif
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_user_configuration_descriptor;

extern const S_usb_device_descriptor uac2_dg8saq_usb_dev_desc;
extern const S_usb_device_descriptor uac2_audio_usb_dev_desc;
#ifdef VDD_SENSE
	extern S_usb_user_configuration_descriptor uac2_usb_conf_desc_fs;
#else
	extern const S_usb_user_configuration_descriptor uac2_usb_conf_desc_fs;
#endif

#if USB_HIGH_SPEED_SUPPORT==ENABLED
	#ifdef VDD_SENSE
		extern S_usb_user_configuration_descriptor uac2_usb_conf_desc_hs;
	#else
		extern const S_usb_user_configuration_descriptor uac2_usb_conf_desc_hs;
	#endif
	extern const S_usb_device_qualifier_descriptor uac2_usb_qualifier_desc;
#endif

#endif  // _UAC2_USB_DESCRIPTORS_H_
