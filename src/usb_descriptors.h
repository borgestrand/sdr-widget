/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* This header file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief USB identifiers.
 *
 * This file contains the USB parameters that uniquely identify the USB
 * application through descriptor tables.
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

#ifndef _USB_DESCRIPTORS_H_
#define _USB_DESCRIPTORS_H_


//_____ I N C L U D E S ____________________________________________________

#include "conf_usb.h"

#if USB_DEVICE_FEATURE == DISABLED
  #error usb_descriptors.h is #included although USB_DEVICE_FEATURE is disabled
#endif


#include "usb_standard_request.h"
#include "usb_task.h"

//_____ functions  ________________________________________________________

extern U8 *__Usb_get_dev_desc_pointer();
extern U16 __Usb_get_dev_desc_length();

//_____ M A C R O S ________________________________________________________

#define Usb_unicode(c)                    (Usb_format_mcu_to_usb_data(16, (U16)(c)))
#define Usb_get_dev_desc_pointer()        __Usb_get_dev_desc_pointer()
#define Usb_get_dev_desc_length()         __Usb_get_dev_desc_length()
#define Usb_get_conf_desc_pointer()       Usb_get_conf_desc_fs_pointer()
#define Usb_get_conf_desc_length()        Usb_get_conf_desc_fs_length()
#define Usb_get_conf_desc_hs_pointer()    (&(usb_conf_desc_hs.cfg.bLength))
#define Usb_get_conf_desc_hs_length()     (sizeof(usb_conf_desc_hs))
#define Usb_get_conf_desc_fs_pointer()    (&(usb_conf_desc_fs.cfg.bLength))
#define Usb_get_conf_desc_fs_length()     (sizeof(usb_conf_desc_fs))
#define Usb_get_qualifier_desc_pointer()  (&(usb_qualifier_desc.bLength))
#define Usb_get_qualifier_desc_length()   (sizeof(usb_qualifier_desc))


//_____ U S B    D E F I N E S _____________________________________________

// USB Device descriptor
#define USB_SPECIFICATION     0x0200
#define DEVICE_CLASS          0xef          //!
#define DEVICE_SUB_CLASS      0x02          //!
#define DEVICE_PROTOCOL       0x01          //! IAD Device
#define EP_CONTROL_LENGTH     64
#define DG8SAQ_VENDOR_ID	  0x16c0
#define DG8SAQ_PRODUCT_ID     0x05dc		//!  DG8SAQ device
#define AUDIO_VENDOR_ID		  0x16c0
#define AUDIO_PRODUCT_ID      0x03e8		//!  Internal Lab use
#define HPSDR_VENDOR_ID       0xfffe		//! Ozy Device
#define HPSDR_PRODUCT_ID      0x0007
#define RELEASE_NUMBER        0x1000
#define MAN_INDEX             0x01
#define PROD_INDEX            0x02
#define SN_INDEX              0x03
#define CLOCK_SOURCE_1_INDEX	  	0x04
#define CLOCK_SOURCE_2_INDEX		0x05
#define CLOCK_SELECTOR_INDEX		0x06


#define WL_INDEX		0x07
#define AIT_INDEX		0x08
#define AOT_INDEX		0x09
#define AIN_INDEX		0x0A
#define AIA_INDEX		0x0B


#define NB_CONFIGURATION      1

// CONFIGURATION
#define NB_INTERFACE	   4	//!  DG8SAQ, Audio (3)
#define CONF_NB            1     //! Number of this configuration
#define CONF_INDEX         0
#define CONF_ATTRIBUTES    USB_CONFIG_SELFPOWERED
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
#define ENDPOINT_NB_1       ( EP_AUDIO_IN | MSK_EP_DIR )
#define EP_ATTRIBUTES_1		0b00100101         // ISOCHROUNOUS ASYNCHRONOUS IMPLICIT FEEDBACK
#define EP_IN_LENGTH_1_FS	294				// 3 bytes * 49 samples * stereo
#define EP_IN_LENGTH_1_HS	294
#define EP_SIZE_1_FS		EP_IN_LENGTH_1_FS
#define EP_SIZE_1_HS        EP_IN_LENGTH_1_HS
#define EP_INTERVAL_1_FS	0x01			 // one packet per uframe, each uF 1ms, so only 48khz
#define EP_INTERVAL_1_HS    0x02			 // One packet per 2 uframe, each uF 125us, so 192khz


// USB Endpoint 2 descriptor
#define ENDPOINT_NB_2       ( EP_AUDIO_OUT )
#define EP_ATTRIBUTES_2     0b00000101			// ISOCHRONOUS ASYNC
#define EP_OUT_LENGTH_2_HS   294				// 3 bytes * 49 samples * stereo
#define EP_OUT_LENGTH_2_FS	294
#define EP_SIZE_2_FS		EP_OUT_LENGTH_2_FS
#define EP_SIZE_2_HS        EP_OUT_LENGTH_2_HS
#define EP_INTERVAL_2_FS	0x01			 // one packet per uframe
#define EP_INTERVAL_2_HS    0x02			 // One packet per 2 uframe


			// USB Endpoint 3 descriptor
#define ENDPOINT_NB_3       ( EP_AUDIO_OUT_FB | MSK_EP_DIR )		// 0x83
#define EP_ATTRIBUTES_3     0b00010001      // ISOCHRONOUS FEEDBACK
#define EP_IN_LENGTH_3_FS   64				//
#define EP_IN_LENGTH_3_HS	64				//
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
#define SPK_INPUT_TERMINAL_CH_NAME_ID	0x00	// No channel name

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
#define STD_AS_INTERFACE_IN				0x02   // Index of Std AS Interface for Audio In
#define STD_AS_INTERFACE_OUT			0x03   // Index of Std AS Interface for Audio Out

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
#define FORMAT_SUBSLOT_SIZE_1				0x03	// Number of bytes per subslot
#define FORMAT_BIT_RESOLUTION_1				0x18	// 24 bits per sample

				//Audio endpoint specific descriptor field
#define AUDIO_EP_ATRIBUTES				0b00000000	 	// No sampling freq, no pitch, no pading
#define AUDIO_EP_CONTROLS				0b00000000
#define AUDIO_EP_DELAY_UNIT				0x00	 	// Unused
#define AUDIO_EP_LOCK_DELAY				0x0000		// Unused

//For playback
#define ALT1_AS_NB_ENDPOINT_OUT			0x02   // two EP,  OUT and OUT_FB
#define AS_TERMINAL_LINK_OUT		    SPK_INPUT_TERMINAL_ID



#define DEVICE_STATUS         SELF_POWERED
#define INTERFACE_STATUS      0x00 // TBD

#define LANG_ID               0x00

#define USB_MN_LENGTH         12
#define USB_MANUFACTURER_NAME \
{\
  Usb_unicode('w'),\
  Usb_unicode('w'),\
  Usb_unicode('w'),\
  Usb_unicode('.'),\
  Usb_unicode('o'),\
  Usb_unicode('b'),\
  Usb_unicode('d'),\
  Usb_unicode('e'),\
  Usb_unicode('v'),\
  Usb_unicode('.'),\
  Usb_unicode('a'),\
  Usb_unicode('t') \
}

#define USB_PN_LENGTH         10
#define USB_PRODUCT_NAME \
{\
  Usb_unicode('D'),\
  Usb_unicode('G'),\
  Usb_unicode('8'),\
  Usb_unicode('S'),\
  Usb_unicode('A'),\
  Usb_unicode('Q'),\
  Usb_unicode('-'),\
  Usb_unicode('I'),\
  Usb_unicode('2'),\
  Usb_unicode('C') \
}

#define USB_SN_LENGTH         13
#define USB_SERIAL_NUMBER \
{\
  Usb_unicode('1'),\
  Usb_unicode('.'),\
  Usb_unicode('0'),\
  Usb_unicode('.'),\
  Usb_unicode('0'),\
  Usb_unicode('.'),\
  Usb_unicode('0'),\
  Usb_unicode('.'),\
  Usb_unicode('0'),\
  Usb_unicode('.'),\
  Usb_unicode('0'),\
  Usb_unicode('.'),\
  Usb_unicode('A') \
}

#define USB_CS1_LENGTH         7
#define USB_CLOCK_SOURCE_1 \
{\
  Usb_unicode('C'),\
  Usb_unicode('l'),\
  Usb_unicode('o'),\
  Usb_unicode('c'),\
  Usb_unicode('k'),\
  Usb_unicode(' '),\
  Usb_unicode('1') \
}

#define USB_CS2_LENGTH         7
#define USB_CLOCK_SOURCE_2 \
{\
  Usb_unicode('C'),\
  Usb_unicode('l'),\
  Usb_unicode('o'),\
  Usb_unicode('c'),\
  Usb_unicode('k'),\
  Usb_unicode(' '),\
  Usb_unicode('2')\
}

#define USB_CX_LENGTH         12
#define USB_CLOCK_SELECTOR \
{\
  Usb_unicode('S'),\
  Usb_unicode('e'),\
  Usb_unicode('l'),\
  Usb_unicode('e'),\
  Usb_unicode('c'),\
  Usb_unicode('t'),\
  Usb_unicode(' '),\
  Usb_unicode('c'),\
  Usb_unicode('l'),\
  Usb_unicode('o'),\
  Usb_unicode('c'),\
  Usb_unicode('k')\
}


#define USB_WL_LENGTH         10
#define USB_WL \
{\
  Usb_unicode('S'),\
  Usb_unicode('D'),\
  Usb_unicode('R'),\
  Usb_unicode('-'),\
  Usb_unicode('W'),\
  Usb_unicode('i'),\
  Usb_unicode('d'),\
  Usb_unicode('g'),\
  Usb_unicode('e'),\
  Usb_unicode('t')\
}
#define USB_AIT_LENGTH         12
#define USB_AIT \
{\
  Usb_unicode('A'),\
  Usb_unicode('u'),\
  Usb_unicode('d'),\
  Usb_unicode('i'),\
  Usb_unicode('o'),\
  Usb_unicode(' '),\
  Usb_unicode('I'),\
  Usb_unicode('n'),\
  Usb_unicode(' '),\
  Usb_unicode('T'),\
  Usb_unicode('r'),\
  Usb_unicode('m')\
}
#define USB_AOT_LENGTH         13
#define USB_AOT \
{\
  Usb_unicode('A'),\
  Usb_unicode('u'),\
  Usb_unicode('d'),\
  Usb_unicode('i'),\
  Usb_unicode('o'),\
  Usb_unicode(' '),\
  Usb_unicode('O'),\
  Usb_unicode('u'),\
  Usb_unicode('t'),\
  Usb_unicode(' '),\
  Usb_unicode('T'),\
  Usb_unicode('r'),\
  Usb_unicode('m')\
}
#define USB_AIN_LENGTH         11
#define USB_AIN \
{\
  Usb_unicode('W'),\
  Usb_unicode('i'),\
  Usb_unicode('d'),\
  Usb_unicode('g'),\
  Usb_unicode('e'),\
  Usb_unicode('t'),\
  Usb_unicode('-'),\
  Usb_unicode('L'),\
  Usb_unicode('i'),\
  Usb_unicode('t'),\
  Usb_unicode('e')\
}
#define USB_AIA_LENGTH         12
#define USB_AIA \
{\
  Usb_unicode('A'),\
  Usb_unicode('u'),\
  Usb_unicode('d'),\
  Usb_unicode('i'),\
  Usb_unicode('o'),\
  Usb_unicode(' '),\
  Usb_unicode('I'),\
  Usb_unicode('n'),\
  Usb_unicode(' '),\
  Usb_unicode('A'),\
  Usb_unicode('l'),\
  Usb_unicode('t')\
}

//_____ U S B  Widget-Lite   D E S C R I P T O R _____________

//! struct usb_WL
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in U8s
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_WL_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_wl;

//_____ U S B  Audio IN Terminal   D E S C R I P T O R _____________

//! struct usb_AIT
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in U8s
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_AIT_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_ait;

//_____ U S B  Audio OUT Terminal   D E S C R I P T O R _____________

//! struct usb_AOT
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in U8s
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_AOT_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_aot;

//_____ U S B  Audio IN   D E S C R I P T O R _____________

//! struct usb_AIN
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in U8s
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_AIN_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_ain;

//_____ U S B  Audio IN Alt   D E S C R I P T O R _____________

//! struct usb_AIA
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in U8s
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_AIA_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_aia;

// Stop test
#define LANGUAGE_ID           0x0409


//! USB Request
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8      bmRequestType;        //!< Characteristics of the request
  U8      bRequest;             //!< Specific request
  U16     wValue;               //!< Field that varies according to request
  U16     wIndex;               //!< Field that varies according to request
  U16     wLength;              //!< Number of bytes to transfer if Data
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_UsbRequest;


//! USB Device Descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8      bLength;              //!< Size of this descriptor in bytes
  U8      bDescriptorType;      //!< DEVICE descriptor type
  U16     bscUSB;               //!< Binay Coded Decimal Spec. release
  U8      bDeviceClass;         //!< Class code assigned by the USB
  U8      bDeviceSubClass;      //!< Subclass code assigned by the USB
  U8      bDeviceProtocol;      //!< Protocol code assigned by the USB
  U8      bMaxPacketSize0;      //!< Max packet size for EP0
  U16     idVendor;             //!< Vendor ID. ATMEL = 0x03EB
  U16     idProduct;            //!< Product ID assigned by the manufacturer
  U16     bcdDevice;            //!< Device release number
  U8      iManufacturer;        //!< Index of manu. string descriptor
  U8      iProduct;             //!< Index of prod. string descriptor
  U8      iSerialNumber;        //!< Index of S.N.  string descriptor
  U8      bNumConfigurations;   //!< Number of possible configurations
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_device_descriptor;


//! USB Configuration Descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8      bLength;              //!< Size of this descriptor in bytes
  U8      bDescriptorType;      //!< CONFIGURATION descriptor type
  U16     wTotalLength;         //!< Total length of data returned
  U8      bNumInterfaces;       //!< Number of interfaces for this conf.
  U8      bConfigurationValue;  //!< Value for SetConfiguration resquest
  U8      iConfiguration;       //!< Index of string descriptor
  U8      bmAttributes;         //!< Configuration characteristics
  U8      MaxPower;             //!< Maximum power consumption
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_configuration_descriptor;


//! USB Interface Descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8      bLength;              //!< Size of this descriptor in bytes
  U8      bDescriptorType;      //!< INTERFACE descriptor type
  U8      bInterfaceNumber;     //!< Number of interface
  U8      bAlternateSetting;    //!< Value to select alternate setting
  U8      bNumEndpoints;        //!< Number of EP except EP 0
  U8      bInterfaceClass;      //!< Class code assigned by the USB
  U8      bInterfaceSubClass;   //!< Subclass code assigned by the USB
  U8      bInterfaceProtocol;   //!< Protocol code assigned by the USB
  U8      iInterface;           //!< Index of string descriptor
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_interface_descriptor;


//! USB Endpoint Descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8      bLength;              //!< Size of this descriptor in bytes
  U8      bDescriptorType;      //!< ENDPOINT descriptor type
  U8      bEndpointAddress;     //!< Address of the endpoint
  U8      bmAttributes;         //!< Endpoint's attributes
  U16     wMaxPacketSize;       //!< Maximum packet size for this EP
  U8      bInterval;            //!< Interval for polling EP in ms
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_endpoint_descriptor;


//! USB Interface Association Descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8 bLength;						// Size of this Descriptor in BYTEs
	U8 bDescriptorType;				// INTERFACE_ASSOCIATION Descriptor Type (0x0B)
	U8 bFirstInterface;				// Interface number of the first one associated with this function
	U8 bInterfaceCount;				// Number of contiguous interface associated with this function
	U8 bFunctionClass;				// The class triad of this interface,
	U8 bFunctionSubClass;			//   usually same as the triad of the first interface
	U8 bFunctionProcotol;
	U8 iInterface;					// Index of String Desc for this function
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_interface_association_descriptor;

//---------------------------------------------
// Class specific descriptors for CDC
//---------------------------------------------
//---------------------------------------------
// Header Functional Descriptor
//---------------------------------------------

typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
    U8 bLength;						// Size of this Descriptor in BYTEs
    U8 bDescriptorType;				// CS_INTERFACE Descriptor Type
    U8 bDescriptorSubtype;			// CS_CDC_HEADER subtype
    U16 bcdCDC;						// bcdCDC (CDC spec release number, 1.1
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_cdc_header_func_descriptor;

//---------------------------------------------
// Call Management Functional Descriptor
//---------------------------------------------
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
    U8 bLength;						// Size of this Descriptor in BYTEs
    U8 bDescriptorType;				// CS_INTERFACE Descriptor Type
    U8 bDescriptorSubtype;			// CS_CDC_CALL_MAN subtype
    U8 bmCapabilities;				// Capabilities bitmap
    U8 bDataInterface;				// Interface number
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_cdc_call_man_func_descriptor;

//---------------------------------------------
// Abstract Control Management Functional Descriptor
//---------------------------------------------
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
    U8 bLength;						// Size of this Descriptor in BYTEs
    U8 bDescriptorType;				// CS_INTERFACE Descriptor Type
    U8 bDescriptorSubtype;			// CS_CDC_ABST_CNTRL subtype
    U8 bmCapabilities;				// Capabilities bitmap
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_cdc_abst_control_mana_descriptor;

//---------------------------------------------
// Union Functional Descriptor
//---------------------------------------------
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8 bLength;						// Size of this Descriptor in BYTEs
	U8 bDescriptorType;				// CS_INTERFACE Descriptor Typ
	U8 bDescriptorSubtype;			// CS_CDC_UNION_FUNC subtype
	U8 bMasterInterface;			// Interface number master
	U8 bSlaveInterface0;			// Interface number slave
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_cdc_union_func_descriptor;



//! USB Device Qualifier Descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8      bLength;              //!< Size of this descriptor in BYTEs
  U8      bDescriptorType;      //!< DEVICE_QUALIFIER descriptor type
  U16     bscUSB;               //!< Binay Coded Decimal Spec. release
  U8      bDeviceClass;         //!< Class code assigned by the USB
  U8      bDeviceSubClass;      //!< Subclass code assigned by the USB
  U8      bDeviceProtocol;      //!< Protocol code assigned by the USB
  U8      bMaxPacketSize0;      //!< Max packet size for EP0
  U8      bNumConfigurations;   //!< Number of possible configurations
  U8      bReserved;            //!< Reserved for future use, must be zero
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_device_qualifier_descriptor;


//! USB Language Descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8      bLength;              //!< Size of this descriptor in U8s
  U8      bDescriptorType;      //!< STRING descriptor type
  U16     wlangid;              //!< Language id
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_language_id;


//_____ U S B   M A N U F A C T U R E R   D E S C R I P T O R _______________

//! struct usb_st_manufacturer
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in BYTEs
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_MN_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_manufacturer_string_descriptor;


//_____ U S B   P R O D U C T   D E S C R I P T O R _________________________

//! struct usb_st_product
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in U8s
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_PN_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_product_string_descriptor;


//_____ U S B   S E R I A L   N U M B E R   D E S C R I P T O R _____________

//! struct usb_st_serial_number
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in U8s
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_SN_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_serial_number;


//_____ U S B   CLOCK SOURCE 1   D E S C R I P T O R _____________

//! struct usb_st_clock_source_1
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in U8s
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_CS1_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_clock_source_1;

//_____ U S B   CLOCK SOURCE 2   D E S C R I P T O R _____________

//! struct usb_st_clock_source_2
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in U8s
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_CS2_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_clock_source_2;

//_____ U S B   CLOCK SELECTOR   D E S C R I P T O R _____________

//! struct usb_st_clock_selector
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;                  //!< Size of this descriptor in U8s
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_CX_LENGTH];   //!< Unicode characters
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_clock_selector;

//_____ U S B   D E V I C E   H I D   D E S C R I P T O R ___________________

//! USB HID Descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8      bLength;              //!< Size of this descriptor in BYTEs
  U8      bDescriptorType;      //!< HID descriptor type
  U16     bcdHID;               //!< HID Class Specification release number
  U8      bCountryCode;         //!< Hardware target country
  U8      bNumDescriptors;      //!< Number of HID class descriptors to follow
  U8      bRDescriptorType;     //!< Report descriptor type
  U16     wItemLength;          //!< Total length of Report descriptor
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_hid_descriptor;



//! A U D I O Class V2.0 Specific paragraph 4.7
//! Audio AC interface descriptor pp 4.7.2
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;               /* Size of this descriptor in bytes */
  U8  bDescriptorType;       /* CS_INTERFACE descriptor type */
  U8 	bDescritorSubtype;     /* HEADER subtype */
  U16 bcdADC;          		  /* Revision of class spec */
  U8  bCategory;				/* Primary use of this function */
  U16 wTotalLength;       	  /* Total size of class specific descriptor */
  U8  bmControls;		     /* Latency Control Bitmap */
} S_usb_ac_interface_descriptor;


//! Clock Source descriptor  pp 4.7.2.1
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;               /* Size of this descriptor in bytes */
  U8  bDescriptorType;       /* CS_INTERFACE descriptor type */
  U8 	bDescritorSubtype;     /* CLOCK_SOURCE subtype */
  U8  bClockID;       	  /* Clock Source ID */
  U8  bmAttributes;		     /* Clock Type Bitmap */
  U8  bmControls;			/* Clock control bitmap */
  U8  bAssocTerminal;		/* Terminal ID associated with this source */
  U8  iClockSource;			/* String descriptor of this clock source */
} S_usb_clock_source_descriptor;


//! Clock Selector descriptor pp 4.7.2.2
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;               /* Size of this descriptor in bytes */
  U8  bDescriptorType;       /* CS_INTERFACE descriptor type */
  U8 	bDescritorSubtype;     /* CLOCK_SELECTOR subtype */
  U8  bClockID;       	  /* Clock Selector ID */
  U8  bNrInPins;		     /* Number of Input Pins */
  U8  baCSourceID1;			/* variable length */
  U8  baCSourceID2;
  U8  bmControls;			/* Clock selector control bitmap  */
  U8  iClockSelector;			/* String descriptor of this clock selector */
} S_usb_clock_selector_descriptor;


//! Clock Multiplier descriptor pp 4.7.2.3
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
  U8  bLength;               /* Size of this descriptor in bytes */
  U8  bDescriptorType;       /* CS_INTERFACE descriptor type */
  U8 	bDescritorSubtype;     /* CLOCK_MULTIPLIER subtype */
  U8  bClockID;       	  /* Clock Multiplier ID */
  U8  bCSourceID;		/* ID of clock entity */
  U8  bmControls;			/* Clock Multiplier control bitmap */
  U8  iClockMultiplier;			/* String descriptor of this clock multiplier */
} S_usb_clock_multiplier_descriptor;


//! USB INPUT Terminal Descriptor pp 4.7.2.4
typedef
#if (defined  __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLength;		/* Size of this descriptor in bytes */
	U8 	bDescriptorType;	/* CS_INTERFACE descriptor type */
	U8 	bDescriptorSubType;	/* INPUT_TERMINAL subtype */
	U8		bTerminalID;	/* Input Terminal ID */
	U16	wTerminalType;		/* Terminal type */
	U8		bAssocTerminal;	/* Output terminal this input is associated with */
	U8		bCSourceID;		/* ID of Clock entity to which this terminal is connected */
	U8		bNrChannels;	/* Number of Logical output channels */
	U32	bmChannelConfig;	/* Spatial location of logical channels */
	U8		iChannelNames;	/* String descriptor of first logical channel */
	U16  bmControls;		/* Paired Bitmap of controls */
	U8		iTerminal;		/* String descriptor of this Input Terminal */
} S_usb_in_ter_descriptor;


//! USB OUTPUT Terminal Descriptor pp 4.7.2.5
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLength;		/* Size of this descriptor in bytes */
	U8 	bDescriptorType;	/* CS_INTERFACE descriptor type */
	U8 	bDescriptorSubType;	/* OUTPUT_TERMINAL subtype */
	U8		bTerminalID;	/* Output Terminal ID */
	U16	wTerminalType;		/* Terminal type */
	U8		bAssocTerminal;	/* Input Terminal this output is associated with */
	U8		bSourceID;		/* ID of the Unit or Terminal to which this teminal is connected to */
	U8      bCSourceID;		/* ID od the Clock Entity to which this terminal is connected */
	U16  bmControls;		/* Paired Bitmap of controls */
	U8		iTerminal;		/* String descriptor of this Output Terminal */
} S_usb_out_ter_descriptor;

//! USB Mixer Unit descriptor pp 4.7.2.6
//! USB Selector Unit descriptor pp 4.7.2.7


//! USB Audio Feature Unit descriptor pp 4.7.2.8
typedef
#if (defined  __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLength;			/* Size of this descriptor in bytes */
	U8 	bDescriptorType;        /* CS_INTERFACE descriptor type */
	U8 	bDescriptorSubType; 	/* FEATURE_UNIT  subtype */
	U8		bUnitID;			/* Feature unit ID */
	U8		bSourceID;			/* ID of the Unit or Terminal to which this teminal is connected to */
	U32		bmaControls;	/* Master Channel 0*/
	U32	    bmaControls_1;  // Channel 1
	U32     bmaControls_2;  // Channel 2
	U8	iFeature;  /* String Descriptor of this Feature Unit */
} S_usb_feature_unit_descriptor;


//! USB Sampling rate Converter Unit descriptor pp 4.7.2.9
//! USB Effects Unit descriptor pp 4.7.2.10
//! USB Parametric Equalizer Section Effect Unit descriptor pp 4.7.2.10.1
//! USB Reverberation Effect Unit descriptor pp 4.7.2.10.2
//! USB Modulation Delay Effect Unit descriptor pp 4.7.2.10.3
//! USB Dynamic Range Compressor Effect Unit descriptor pp 4.7.2.10.4
//! USB Processing Unit descriptor pp 4.7.2.11
//! USB Up/Down-mix Processing Unit descriptor pp 4.7.2.11.1
//! USB Dolby Prologic Processing Unit descriptor pp 4.7.2.11.2
//! USB Stereo Extender Processing Unit descriptor pp 4.7.2.11.3
//! USB Extension Unit descriptor pp 4.7.2.12

//! Audio Contro Endpoint Descriptors pp 4.8
//! USB AC Control Endpoint Descriptors pp 4.8.1
//! Standard AC Control Endpoint Descriptor pp 4.8.1.1
//! Class-Specific AC Control Endpoint Descriptor pp 4.8.1.2
//! AC Interrupt Endpoint Descriptors pp 4.8.2
//! Standard AC Interrupt Endpoint Descriptors pp 4.8.2.1
//! Class-Specific AC Interrupt Endpoint Descriptor pp 4.8.2.2


//! Audio Streaming Interface Descriptors pp 4.9

//! USB Standard AS interface Descriptor pp 4.9.1
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLength;			/* Size of this descriptor in bytes */
	U8 		bDescriptorType;		/* INTERFACE descriptor type */
	U8		bInterfaceNumber;	/* Number of the interface (0 based) */
	U8		bAlternateSetting;
	U8		bNumEndpoints;		/* Number of endpoints in this interface */
	U8		bInterfaceClass;	/* AUDIO Interface class code */
	U8		bInterfaceSubclass;	/* AUDIO_STREAMING Interface subclass code */
	U8		bInterfaceProtocol;	/* IP_VERSION_02_00 Interface protocol code */
	U8		iInterface;			/* String descriptor of this Interface */
} S_usb_as_interface_descriptor;


//! USB Class-Specific AS general interface descriptor pp 4.9.2
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLength;			/* Size of this descriptor in bytes */
	U8 	bDescriptorType;		/* CS_INTERFACE descriptor type */
	U8 	bDescriptorSubType;		/* AS_GENERAL subtype */
	U8		bTerminalLink;		/* Terminal ID to which this interface is connected */
	U8  	bmControls;			/* Bitmap of controls */
	U8  bFormatType;			/* Format type the interface is using */
	U32 bmFormats;				/* Bitmap of Formats this interface supports */
	U8  bNrChannels;			/* Number of Physical channels in this interface cluster */
	U32 bmChannelConfig;		/* Bitmap of spatial locations of the physical channels */
	U8  iChannelNames;			/* String descriptor of the first physical channel */
} S_usb_as_g_interface_descriptor;


//! Class-Specific Audio Format Type descriptor pp 4.9.3 -> 2.3.1.6 Type I Format
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLength;			/* Size of this descriptor in bytes */
	U8 	bDescriptorType;		/* CS_INTERFACE descriptor type */
	U8 	bDescriptorSubType;		/* FORMAT_TYPE subtype */
	U8		bFormatType;		/* Format Type this streaming interface is using */
	U8		bSubslotSize;		/* Number of bytes in one audio subslot */
	U8		bBitResolution;		/* Number of bits used from the available bits in the subslot */
} S_usb_format_type;


//! USB Class-Specific AS Encoder Descriptor pp 4.9.4
//! USB Encoder Descriptor pp 4.9.4.1
//! USB Class-Specific AS Decoder Descriptor pp 4.9.5
//! USB MPEG Decoder Descriptor pp 4.9.5.1
//! USB AC-3 Decoder Descriptor pp 4.9.5.2
//! USB WMA Decoder Descriptor pp 4.9.5.3
//! USB DTS Decoder Descriptor pp 4.9.5.4


//! Usb Standard AS Isochronous Audio Data Endpoint Descriptors pp 4.10.1.1
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
   U8      bLength;               /* Size of this descriptor in bytes */
   U8      bDescriptorType;       /* CS_ENDPOINT descriptor type */
   U8      bEndpointAddress;      /* Address of the endpoint */
   U8      bmAttributes;          /* Endpoint's attributes */
   U16     wMaxPacketSize;        /* Maximum packet size for this EP */
   U8      bInterval;             /* Interval for polling EP in ms */
} S_usb_endpoint_audio_descriptor;


//! Usb Class_Specific AS Isochronous Audio Data Endpoint Descriptors pp 4.10.1.2
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLength;			/* Size of this descriptor in bytes */
	U8 		bDescriptorType;	/* CS_ENDPOINT descriptor type*/
	U8 		bDescriptorSubType;	/* EP_GENERAL subtype */
	U8		bmAttributes;		/* Bitmap of attributes 8 */
	U8      bmControls;			/* Paired bitmap of controls */
	U8    	bLockDelayUnits;		/* units for wLockDelay */
	U16		wLockDelay;				/* time to lock endpoint */
} S_usb_endpoint_audio_specific;

//! Usb Standard AS Isochronous Feedback Endpoint Descriptors pp 4.10.2.1
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLength;			/* Size of this descriptor in bytes */
	U8 		bDescriptorType;	/* ENDPOINT descriptor type*/
	U8      bEndpointAddress;	/* Endpoint Address */
	U8		bmAttributes;		/* Bitmap of attributes 8 */
	U8      bmControls;			/* Paired bitmap of controls */
	U16     wMaxPacketSize;     /* Maximum packet size for this EP */
	U8      bInterval;          /* Interval for polling EP in ms */
} S_usb_endpoint_audio_feedback;

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
S_usb_configuration_descriptor 						cfg;
	S_usb_interface_descriptor	 					ifc0;

//! Audio descriptors Class 2

			S_usb_interface_association_descriptor 		iad1;
			S_usb_interface_descriptor					ifc1;
			S_usb_ac_interface_descriptor			audioac;
			S_usb_clock_source_descriptor			audio_cs1;
			S_usb_clock_source_descriptor			audio_cs2;
			S_usb_clock_selector_descriptor			audio_csel;
//			S_usb_clock_multiplier_descriptor		audio_cmul;
			S_usb_in_ter_descriptor 				mic_in_ter;
			S_usb_feature_unit_descriptor 			mic_fea_unit;
			S_usb_out_ter_descriptor				mic_out_ter;
			S_usb_in_ter_descriptor					spk_in_ter;
			S_usb_feature_unit_descriptor			spk_fea_unit;
			S_usb_out_ter_descriptor				spk_out_ter;
			S_usb_as_interface_descriptor	 		mic_as_alt0;
			S_usb_as_interface_descriptor	 		mic_as_alt1;
			S_usb_as_g_interface_descriptor			mic_g_as;
			S_usb_format_type						mic_format_type;
			S_usb_endpoint_audio_descriptor 		ep1;
			S_usb_endpoint_audio_specific 			ep1_s;
			S_usb_as_interface_descriptor	 		spk_as_alt0;
			S_usb_as_interface_descriptor	 		spk_as_alt1;
			S_usb_as_g_interface_descriptor			spk_g_as;
			S_usb_format_type						spk_format_type;
			S_usb_endpoint_audio_descriptor 		ep2;
			S_usb_endpoint_audio_specific 			ep2_s;
			S_usb_endpoint_audio_descriptor 		ep3;
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_user_configuration_descriptor;


#endif  // _USB_DESCRIPTORS_H_
