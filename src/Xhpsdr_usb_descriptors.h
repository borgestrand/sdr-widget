/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* This header file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

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

#ifndef _HPSDR_USB_DESCRIPTORS_H_
#define _HPSDR_USB_DESCRIPTORS_H_


//_____ I N C L U D E S ____________________________________________________

#include "conf_usb.h"
#include "usb_descriptors.h"

#if USB_DEVICE_FEATURE == DISABLED
  #error hpsdr_usb_descriptors.h is #included although USB_DEVICE_FEATURE is disabled
#endif


#include "usb_standard_request.h"
#include "usb_task.h"

//_____ U S B    D E F I N E S _____________________________________________


// CONFIGURATION
#define NB_INTERFACE	   1	//!  Number of Interface
#define CONF_NB            1     //! Number of this configuration
#define CONF_INDEX         0
#define CONF_ATTRIBUTES    USB_CONFIG_SELFPOWERED
#define MAX_POWER          5 // 250    // 500 mA


// USB Interface descriptor for Ozy
#define INTERFACE_NB0			    0
#define ALTERNATE_NB0	            0                  //! The alt setting nb of this interface
#define NB_ENDPOINT0			    3                  //! The number of endpoints this interface has
#define INTERFACE_CLASS0		    NO_CLASS          //! No Class
#define INTERFACE_SUB_CLASS0        NO_SUBCLASS        //! No Subclass
#define INTERFACE_PROTOCOL0    		NO_PROTOCOL		   //! No Protocol
#define INTERFACE_INDEX0       		0

#define DSC_INTERFACE_OZY			INTERFACE_NB0


// USB Endpoint 1 descriptor
#define ENDPOINT_NB_1       ( HPSDR_EP_RF_IN | MSK_EP_DIR )
#define EP_ATTRIBUTES_1		TYPE_BULK
#define EP_IN_LENGTH_1_FS	512
#define EP_IN_LENGTH_1_HS	512
#define EP_SIZE_1_FS		EP_IN_LENGTH_1_FS
#define EP_SIZE_1_HS        EP_IN_LENGTH_1_HS
#define EP_INTERVAL_1_FS	0x01			 // one packet per frame, each uF 1ms, so only 48khz
#define EP_INTERVAL_1_HS    0x04			 // One packet per 4 uframe, each uF 125us, so 192khz


// USB Endpoint 2 descriptor
#define ENDPOINT_NB_2       ( HPSDR_EP_IQ_IN | MSK_EP_DIR )
#define EP_ATTRIBUTES_2     TYPE_BULK
#define EP_OUT_LENGTH_2_FS	64
#define EP_OUT_LENGTH_2_HS  512
#define EP_SIZE_2_FS		EP_OUT_LENGTH_2_FS
#define EP_SIZE_2_HS        EP_OUT_LENGTH_2_HS
#define EP_INTERVAL_2_FS	0x00
#define EP_INTERVAL_2_HS    0x00


// USB Endpoint 3 descriptor
#define ENDPOINT_NB_3       (HPSDR_EP_IQ_OUT)
#define EP_ATTRIBUTES_3     TYPE_BULK
#define EP_IN_LENGTH_3_FS   64
#define EP_IN_LENGTH_3_HS	512
#define EP_SIZE_3_FS		EP_IN_LENGTH_3_FS
#define EP_SIZE_3_HS        EP_IN_LENGTH_3_HS
#define EP_INTERVAL_3_FS	0x00
#define EP_INTERVAL_3_HS    0x00

typedef
#if (defined __ICCAVR32__)
#pragma pack(1)
#endif
struct
#if (defined __GNUC__)
__attribute__((__packed__))
#endif
{
	S_usb_configuration_descriptor					cfg;
//	S_usb_interface_descriptor	 					ifc0;
	S_usb_endpoint_descriptor						ep1;
	S_usb_endpoint_descriptor						ep2;
	S_usb_endpoint_descriptor						ep3;
}
#if (defined __ICCAVR32__)
#pragma pack()
#endif
S_usb_user_configuration_descriptor;

extern const S_usb_device_descriptor hpsdr_usb_dev_desc;
extern const S_usb_user_configuration_descriptor hpsdr_usb_conf_desc_fs;
#if (USB_HIGH_SPEED_SUPPORT==ENABLED)
extern const S_usb_user_configuration_descriptor hpsdr_usb_conf_desc_hs;
extern const S_usb_device_qualifier_descriptor hpsdr_usb_qualifier_desc;
#endif
extern const S_usb_product_string_descriptor hpsdr_usb_user_product_string_descriptor;

#endif  // _HPSDR_USB_DESCRIPTORS_H_
