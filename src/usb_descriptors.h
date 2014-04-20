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
 * Copyright (C) 2010 Alex Lee
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
 * For sdr-widget and audio-widget, custom boards based on the AT32UC3A3256
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
#include "image.h"

//_____ functions  ________________________________________________________

//_____ M A C R O S ________________________________________________________

#define Usb_unicode(c)                    (Usb_format_mcu_to_usb_data(16, (U16)(c)))
#define Usb_get_dev_desc_pointer()        image_get_dev_desc_pointer()
#define Usb_get_dev_desc_length()         image_get_dev_desc_length()
#define Usb_get_conf_desc_pointer()       image_get_conf_desc_fs_pointer()
#define Usb_get_conf_desc_length()        image_get_conf_desc_fs_length()
#define Usb_get_conf_desc_hs_pointer()    image_get_conf_desc_hs_pointer()
#define Usb_get_conf_desc_hs_length()     image_get_conf_desc_hs_length()
#define Usb_get_conf_desc_fs_pointer()    image_get_conf_desc_fs_pointer()
#define Usb_get_conf_desc_fs_length()     image_get_conf_desc_fs_length()
#define Usb_get_qualifier_desc_pointer()  image_get_qualifier_desc_pointer()
#define Usb_get_qualifier_desc_length()   image_get_qualifier_desc_length()


//_____ U S B    D E F I N E S _____________________________________________

// USB Device descriptor

// BSB Added 20110901 according to mail from Roger
#define USB_1_1_SPECIFICATION     0x0110 	// BSB 20130605 changed from 0x0101 to 0x0110 to happify USBlyzer

#define USB_SPECIFICATION     0x0200
#define DEVICE_CLASS          0xef          //!
#define DEVICE_SUB_CLASS      0x02          //!
#define DEVICE_PROTOCOL       0x01          //! IAD Device
#define EP_CONTROL_LENGTH     64
#define DG8SAQ_VENDOR_ID      0x16c0        //!  DG8SAQ device
#define DG8SAQ_PRODUCT_ID     0x05dc

#ifdef COMPILING_FOR_DRIVER_DEVELOPMENT
  // "internal lab use only" VID=0x16C0
  // UAC1 PID=0x03ed, UAC2 PID is one above.
  // Hence use increment of 2 if Host computer needs new PID
  // to continue debugging

  #ifndef FEATURE_PRODUCT_AB1x
  #define FEATURE_PRODUCT_AB1x
  #endif

  #define AUDIO_VENDOR_ID		0x16c0
  #define AUDIO_PRODUCT_ID_9		0x03ed			// UAC1 PID
  #define AUDIO_PRODUCT_ID_10		AUDIO_PRODUCT_ID_9 + 1	// UAC2 PID
#else
  // Use product-specific VID/PIDs
  #if defined(FEATURE_PRODUCT_SDR_WIDGET)
    #define AUDIO_VENDOR_ID       0x16d0
    #define AUDIO_PRODUCT_ID_1    0x0761	//!  SDR-WIDGET	UAC1 PID
    #define AUDIO_PRODUCT_ID_2    0x0762	//!  SDR-WIDGET	UAC2 PID
  #elif (defined(FEATURE_PRODUCT_USB9023))
    #define AUDIO_VENDOR_ID       0x16d0	//!  USB9023	VID
    #define AUDIO_PRODUCT_ID_3    0x0763	//!  USB9023	UAC1 PID
    #define AUDIO_PRODUCT_ID_4    0x0764	//!  USB9023	UAC2 PID
  #elif (defined(FEATURE_PRODUCT_USB5102))
    #define AUDIO_VENDOR_ID       0x16d0	//!  USB5102	VID
    #define AUDIO_PRODUCT_ID_5    0x0765	//!  USB5102	UAC1 PID
    #define AUDIO_PRODUCT_ID_6    0x0766	//!  USB5102	UAC2 PID
  #elif (defined(FEATURE_PRODUCT_USB8741))
    #define AUDIO_VENDOR_ID       0x16d0	//!  USB8741	VID
    #define AUDIO_PRODUCT_ID_7	  0x0767	//!  USB8741	UAC1 PID
    #define AUDIO_PRODUCT_ID_8    0x0768	//!  USB8741	UAC2 PID
  #elif (defined(FEATURE_PRODUCT_AB1x))
    #define AUDIO_VENDOR_ID       0x16d0	//!  AB-1.x	VID
    #define AUDIO_PRODUCT_ID_9    0x075c	//!  AB-1.x	UAC1 PID
    #define AUDIO_PRODUCT_ID_10   0x075d	//!  AB-1.x	UAC2 PID
  #elif (defined(FEATURE_PRODUCT_QNKTC_FUTURE))
    #define AUDIO_VENDOR_ID       0x16d0	//!  AB-1.x	VID
    #define AUDIO_PRODUCT_ID_11   0x075e	//!  QNKTC future use UAC1 PID
    #define AUDIO_PRODUCT_ID_12   0x075f	//!  QNKTC future use UAC2 PID
  #elif (defined(FEATURE_PRODUCT_AMB))
    #define AUDIO_VENDOR_ID       0x16d0	//!  AMB	VID
    #define AUDIO_PRODUCT_ID_13   0x098b	//!  AMB	UAC1 PID
    #define AUDIO_PRODUCT_ID_14   0x098c	//!  AMB	UAC2 PID
  #else
    #error No recognized FEATURE_PRODUCT... is defined in Makefile, aborting.
  #endif
#endif	// DRIVER_DEVELOPMENT

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

// image specific information moved to *_usb_descriptors.h

#define DEVICE_STATUS         SELF_POWERED
#define INTERFACE_STATUS      0x00 // TBD

#define LANG_ID               0x00

// BSB 20120928 new VID/PID system
#if defined (FEATURE_PRODUCT_SDR_WIDGET) // AUDIO_PRODUCT_ID_1 and _2
#define USB_MN_LENGTH         10
#define USB_MANUFACTURER_NAME {\
  Usb_unicode('S'), Usb_unicode('D'), Usb_unicode('R'), Usb_unicode('-'), Usb_unicode('W'),\
  Usb_unicode('i'), Usb_unicode('d'), Usb_unicode('g'), Usb_unicode('e'), Usb_unicode('t')\
}
#elif defined (FEATURE_PRODUCT_USB9023) // AUDIO_PRODUCT_ID_3 and _4
#define USB_MN_LENGTH         12
#define USB_MANUFACTURER_NAME {\
  Usb_unicode('A'), Usb_unicode('u'), Usb_unicode('d'), Usb_unicode('i'), Usb_unicode('o'), Usb_unicode('-'), \
  Usb_unicode('W'), Usb_unicode('i'), Usb_unicode('d'), Usb_unicode('g'), Usb_unicode('e'), Usb_unicode('t')\
}
#elif defined (FEATURE_PRODUCT_USB5102) // AUDIO_PRODUCT_ID_5 and _6
#define USB_MN_LENGTH         12
#define USB_MANUFACTURER_NAME {\
  Usb_unicode('A'), Usb_unicode('u'), Usb_unicode('d'), Usb_unicode('i'), Usb_unicode('o'), Usb_unicode('-'), \
  Usb_unicode('W'), Usb_unicode('i'), Usb_unicode('d'), Usb_unicode('g'), Usb_unicode('e'), Usb_unicode('t')\
}
#elif defined (FEATURE_PRODUCT_USB8741) // AUDIO_PRODUCT_ID_7 and _8
#define USB_MN_LENGTH         12
#define USB_MANUFACTURER_NAME {\
  Usb_unicode('A'), Usb_unicode('u'), Usb_unicode('d'), Usb_unicode('i'), Usb_unicode('o'), Usb_unicode('-'), \
  Usb_unicode('W'), Usb_unicode('i'), Usb_unicode('d'), Usb_unicode('g'), Usb_unicode('e'), Usb_unicode('t')\
}
#elif defined (FEATURE_PRODUCT_AB1x)  // AUDIO_PRODUCT_ID_9 and _10
#define USB_MN_LENGTH         12
#define USB_MANUFACTURER_NAME {\
  Usb_unicode('A'), Usb_unicode('u'), Usb_unicode('d'), Usb_unicode('i'), Usb_unicode('o'), Usb_unicode('-'), \
  Usb_unicode('W'), Usb_unicode('i'), Usb_unicode('d'), Usb_unicode('g'), Usb_unicode('e'), Usb_unicode('t')\
}
#elif defined (FEATURE_PRODUCT_AMB)  // AUDIO_PRODUCT_ID_13 and _14
#define USB_MN_LENGTH         16
#define USB_MANUFACTURER_NAME {\
  Usb_unicode('A'), Usb_unicode('M'), Usb_unicode('B'), Usb_unicode(' '), Usb_unicode('L'), Usb_unicode('a'), \
  Usb_unicode('b'), Usb_unicode('o'), Usb_unicode('r'), Usb_unicode('a'), Usb_unicode('t'), Usb_unicode('o'), \
  Usb_unicode('r'), Usb_unicode('i'), Usb_unicode('e'), Usb_unicode('s')\
}
#else
#error No recognized FEATURE_PRODUCT... is defined in Makefile, aborting.
#endif


// BSB 20120928 new VID/PID system
#if defined (FEATURE_PRODUCT_SDR_WIDGET) // AUDIO_PRODUCT_ID_1 and _2
  #define USB_PN_LENGTH         19
  #define USB_PRODUCT_NAME {\
    Usb_unicode('Y'), Usb_unicode('o'), Usb_unicode('y'), Usb_unicode('o'), Usb_unicode('d'),\
    Usb_unicode('y'), Usb_unicode('n'), Usb_unicode('e'), Usb_unicode(' '), \
    Usb_unicode('S'), Usb_unicode('D'), Usb_unicode('R'), Usb_unicode('-'), Usb_unicode('W'),\
    Usb_unicode('i'), Usb_unicode('d'), Usb_unicode('g'), Usb_unicode('e'), Usb_unicode('t')\
  }
#elif defined (FEATURE_PRODUCT_USB9023) // AUDIO_PRODUCT_ID_3 and _4
  #define USB_PN_LENGTH         16
  #define USB_PRODUCT_NAME {\
    Usb_unicode('Y'), Usb_unicode('o'), Usb_unicode('y'), Usb_unicode('o'), Usb_unicode('d'),\
    Usb_unicode('y'), Usb_unicode('n'), Usb_unicode('e'), Usb_unicode(' '), Usb_unicode('U'),\
    Usb_unicode('S'), Usb_unicode('B'), Usb_unicode('9'), Usb_unicode('0'), Usb_unicode('2'), Usb_unicode('3')\
  }
#elif defined (FEATURE_PRODUCT_USB5102) // AUDIO_PRODUCT_ID_5 and _6
  #define USB_PN_LENGTH         16
  #define USB_PRODUCT_NAME {\
    Usb_unicode('Y'), Usb_unicode('o'), Usb_unicode('y'), Usb_unicode('o'), Usb_unicode('d'),\
    Usb_unicode('y'), Usb_unicode('n'), Usb_unicode('e'), Usb_unicode(' '), Usb_unicode('U'),\
    Usb_unicode('S'), Usb_unicode('B'), Usb_unicode('5'), Usb_unicode('1'), Usb_unicode('0'), Usb_unicode('2')\
 }
#elif defined (FEATURE_PRODUCT_USB8741) // AUDIO_PRODUCT_ID_7 and _8
  #define USB_PN_LENGTH         16
  #define USB_PRODUCT_NAME {\
    Usb_unicode('Y'), Usb_unicode('o'), Usb_unicode('y'), Usb_unicode('o'), Usb_unicode('d'),\
    Usb_unicode('y'), Usb_unicode('n'), Usb_unicode('e'), Usb_unicode(' '), Usb_unicode('U'),\
    Usb_unicode('S'), Usb_unicode('B'), Usb_unicode('8'), Usb_unicode('7'), Usb_unicode('4'), Usb_unicode('1')\
  }
#elif defined (FEATURE_PRODUCT_AB1x)  // AUDIO_PRODUCT_ID_9 and _10
  #ifdef COMPILING_FOR_DRIVER_DEVELOPMENT
    #define USB_PN_LENGTH         20
    #define USB_PRODUCT_NAME {\
      Usb_unicode('D'), Usb_unicode('r'), Usb_unicode('i'), Usb_unicode('v'), Usb_unicode('e'), Usb_unicode('r'), \
      Usb_unicode(' '), Usb_unicode('D'), Usb_unicode('e'), Usb_unicode('v'), Usb_unicode('e'), Usb_unicode('l'), \
      Usb_unicode('.'), Usb_unicode(' '), Usb_unicode('A'), Usb_unicode('B'), Usb_unicode('-'), Usb_unicode('1'), \
      Usb_unicode('.'), Usb_unicode('2')\
    }
  #else
    #define USB_PN_LENGTH         20
    #define USB_PRODUCT_NAME {\
      Usb_unicode('Q'), Usb_unicode('N'), Usb_unicode('K'), Usb_unicode('T'), Usb_unicode('C'), Usb_unicode(' '), \
      Usb_unicode('U'), Usb_unicode('S'), Usb_unicode('B'), Usb_unicode(' '), Usb_unicode('D'), Usb_unicode('A'), \
      Usb_unicode('C'), Usb_unicode(' '), Usb_unicode('A'), Usb_unicode('B'), Usb_unicode('-'), Usb_unicode('1'), \
      Usb_unicode('.'), Usb_unicode('2')\
    }
  #endif
#elif defined (FEATURE_PRODUCT_AMB)  // AUDIO_PRODUCT_ID_13 and _14
    #define USB_PN_LENGTH         24
    #define USB_PRODUCT_NAME {\
      Usb_unicode('A'), Usb_unicode('M'), Usb_unicode('B'), Usb_unicode(' '), Usb_unicode('U'), Usb_unicode('S'), \
      Usb_unicode('B'), Usb_unicode(' '), Usb_unicode('D'), Usb_unicode('A'), Usb_unicode('C'), Usb_unicode(' '), \
      Usb_unicode('A'), Usb_unicode('u'), Usb_unicode('d'), Usb_unicode('i'), Usb_unicode('o'), Usb_unicode('-'), \
      Usb_unicode('W'), Usb_unicode('i'), Usb_unicode('d'), Usb_unicode('g'), Usb_unicode('e'), Usb_unicode('t') \
    }
#else
#error No recognized FEATURE_PRODUCT... is defined in Makefile, aborting.
#endif


#define HPSDR_USB_PN_LENGTH         9
#define HPSDR_USB_PRODUCT_NAME \
{\
  Usb_unicode('O'),\
  Usb_unicode('Z'),\
  Usb_unicode('Y'),\
  Usb_unicode(' '),\
  Usb_unicode('C'),\
  Usb_unicode('L'),\
  Usb_unicode('O'),\
  Usb_unicode('N'),\
  Usb_unicode('E')\
}

#define USB_SN_LENGTH         13
#define USB_SERIAL_NUMBER \
{\
  Usb_unicode('2'),\
  Usb_unicode('0'),\
  Usb_unicode('1'),\
  Usb_unicode('4'),\
  Usb_unicode('0'),\
  Usb_unicode('4'),\
  Usb_unicode('1'),\
  Usb_unicode('8'),\
  Usb_unicode('0'),\
  Usb_unicode('0'),\
  Usb_unicode('A'),\
  Usb_unicode('M'),\
  Usb_unicode('B') \
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

// Revised for Mac UAC1 naming
#define USB_WL_LENGTH USB_PN_LENGTH
#define USB_WL USB_PRODUCT_NAME
/*
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
*/


#define USB_AIT_LENGTH         12
#define USB_AIT \
{\
  Usb_unicode('A'),\
  Usb_unicode('u'),\
  Usb_unicode('d'),\
  Usb_unicode('i'),\
  Usb_unicode('o'),\
  Usb_unicode('-'),\
  Usb_unicode('w'),\
  Usb_unicode('i'),\
  Usb_unicode('d'),\
  Usb_unicode('g'),\
  Usb_unicode('e'),\
  Usb_unicode('t')\
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



//! A U D I O Specific 
//! Audio AC interface descriptor
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
  U8  bDescriptorType;       /* CS interface*/
  U8  bDescritorSubtype;     /* HEADER Subtype */
  U16 bcdADC;          		  /* Revision of class spec */
  U16 wTotalLength;       	  /* Total size of class specific descriptor */
  U8  bInCollection;         /* Number of streaming interface */
  U8  baInterfaceNr0;		     /* Streaming interface number 0*/
  // BSB 20130604 disabling UAC1 IN   U8  baInterfaceNr1;		// Streaming interface number 1
} S_usb_ac_interface_descriptor_1;

//! USB INPUT Terminal Descriptor
typedef
#if (defined  __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLenght;
	U8 	bDescriptorType;
	U8 	bDescriptorSubType;
	U8		bTerminalID;
	U16	wTerminalType;
	U8		bAssocTerminal;
	U8		bNrChannels;
	U16	wChannelConfig;
	U8		iChannelNames;
	U8		iTerminal;
} S_usb_in_ter_descriptor_1;


//! USB Audio Feature Unit descriptor
typedef
#if (defined  __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLenght;
	U8 	bDescriptorType;
	U8 	bDescriptorSubType;
	U8		bUnitID;
	U8    bSourceID;
	U8		bControSize;
	U16	bmaControls_0;
	U16	bmaControls_1;
	U16	bmsCnotrols_2;
	U8	iTerminal;
} S_usb_feature_unit_descriptor_1;

//! USB OUTPUT Terminal Descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLenght;
	U8 	bDescriptorType;
	U8 	bDescriptorSubType;
	U8		bTerminalID;
	U16	wTerminalType;
	U8		bAssocTerminal;
	U8		bSourceID;
	U8		iTerminal;
} S_usb_out_ter_descriptor_1;


//! USB Standard AS interface Descriptor
// common across 1 and 2
// typedef
// #if (defined __ICCAVR32__)
// #pragma pack(1)
// #endif
// struct
// #if (defined __GNUC__)
// __attribute__((__packed__))
// #endif
// {
// 	U8		bLenght;
// 	U8 	bDescriptorType;
// 	U8		bInterfaceNumber;
// 	U8		bAlternateSetting;
// 	U8		bNumEndpoints;
// 	U8		bInterfaceClass;
// 	U8		bInterfaceSubclass;
// 	U8		bInterfaceProtocol;
// 	U8		iInterface;
// } S_usb_as_interface_descriptor_1;


//! USB AS general interface descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLenght;
	U8 	bDescriptorType;
	U8 	bDescriptorSubType;
	U8		bTerminalLink;
	U8    bDelay;
	U16	wFormatTag;
} S_usb_as_g_interface_descriptor_1;


//! Audio Format Type descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLenght;
	U8 	bDescriptorType;
	U8 	bDescriptorSubType;
	U8		bFormatType;
	U8		bNrChannels;
	U8		bSubFrameSize;
	U8		bBitResolution;
	U8		bSampleFreqType;
	U16	wLsbyteiSamFreq_1;
	U8		bMsbyteiSamFreq_1;
	U16	wLsbyteiSamFreq_2;
	U8		bMsbyteiSamFreq_2;
} S_usb_format_type_1;


//! Endpoint AUDIO Specific descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	U8		bLenght;
	U8 	bDescriptorType;
	U8 	bDescriptorSubType;
	U8		bmAttributes;
	U8    bLockDelayUnits;
	U16	wLockDelay;
}S_usb_endpoint_audio_specific_1;

//! Usb Audio Endpoint Descriptor
typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
   U8      bLength;               //!< Size of this descriptor in bytes
   U8      bDescriptorType;       //!< ENDPOINT descriptor type
   U8      bEndpointAddress;      //!< Address of the endpoint
   U8      bmAttributes;          //!< Endpoint's attributes
   U16     wMaxPacketSize;        //!< Maximum packet size for this EP
   U8      bInterval;             //!< Interval for polling EP in ms
	U8		  bRefresh;
	U8		  bSynAddress;
} S_usb_endpoint_audio_descriptor_1;


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
  U8  bDescritorSubtype;     /* HEADER subtype */
  U16 bcdADC;          		  /* Revision of class spec */
  U8  bCategory;				/* Primary use of this function */
  U16 wTotalLength;       	  /* Total size of class specific descriptor */
  U8  bmControls;		     /* Latency Control Bitmap */
} S_usb_ac_interface_descriptor_2;


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
} S_usb_in_ter_descriptor_2;


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
} S_usb_out_ter_descriptor_2;

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
} S_usb_feature_unit_descriptor_2;


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
} S_usb_as_g_interface_descriptor_2;


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
} S_usb_format_type_2;


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
} S_usb_endpoint_audio_descriptor_2;


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
} S_usb_endpoint_audio_specific_2;

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

// task specific structure definitions moved to *_usb_descriptors.h

extern const S_usb_manufacturer_string_descriptor usb_user_manufacturer_string_descriptor;
extern const S_usb_product_string_descriptor usb_user_product_string_descriptor;
extern const S_usb_serial_number usb_user_serial_number;
extern const S_usb_language_id usb_user_language_id;
extern const S_usb_clock_source_1 usb_user_clock_source_1;
extern const S_usb_clock_source_2 usb_user_clock_source_2;
extern const S_usb_clock_selector usb_user_clock_selector;

extern const S_usb_wl usb_user_wl;
extern const S_usb_ait usb_user_ait;
extern const S_usb_aot usb_user_aot;
extern const S_usb_ain usb_user_ain;
extern const S_usb_aia usb_user_aia;

// #define USB_HID_REPORT_DESC 47
#define USB_HID_REPORT_DESC 67 // BSB 20120711: Changed according to BasicAudioDevice-10.pdf table 8-2
extern const U8 usb_hid_report_descriptor[USB_HID_REPORT_DESC];

extern U16 configTSK_USB_DEV_PERIOD;

#endif  // _USB_DESCRIPTORS_H_
