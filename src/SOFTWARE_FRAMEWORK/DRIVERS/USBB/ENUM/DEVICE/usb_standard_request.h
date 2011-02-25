/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* This header file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Processing of USB device enumeration requests.
 *
 * This file contains the USB control endpoint management
 * routines corresponding to the standard enumeration process (refer to
 * chapter 9 of the USB specification).
 * This file calls routines of the usb_specific_request.c file for
 * non-standard request management.
 * The enumeration parameters (descriptor tables) are contained in the
 * usb_descriptors.c file.
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
 * Modified by Alex Lee & SDR-Widget Team for SDR-Widget 22 Apr 2010
 */

#ifndef _USB_STANDARD_REQUEST_H_
#define _USB_STANDARD_REQUEST_H_


//_____ I N C L U D E S ____________________________________________________

#include "conf_usb.h"

#if USB_DEVICE_FEATURE == DISABLED
  #error usb_standard_request.h is #included although USB_DEVICE_FEATURE is disabled
#endif


#include "usb_task.h"
#include "usb_descriptors.h"


//-----------------------------------------------------------------------------
// Definition of Standard device request
//-----------------------------------------------------------------------------

// Device Request Direction (bmRequestType bit7)
#define DRD_MASK				0x80		// Mask for device request direction
#define DRD_OUT					0x00		// OUT: host to device
#define DRD_IN					0x80		// IN:	device to host

// Device Request Type (bmRequestType bit6-5)
#define DRT_MASK				0x60		// Mask for device request type
#define DRT_STD					0x00		// Standard device request
#define DRT_CLASS				0x20		// Class specific request
#define DRT_VENDOR				0x40		// Vendor specific request

// Device Request Recipient (bmRequestType bit4-0)
#define DRR_MASK				0x1F		// Mask for device request recipient
#define DRR_DEVICE				0x00		// Device
#define DRR_INTERFACE			0x01		// Interface
#define DRR_ENDPOINT			0x02		// Endpoint

// Define bmRequestType bitmaps
#define OUT_DEVICE			(DRD_OUT | DRT_STD | DRR_DEVICE)		// Request made to device,
#define IN_DEVICE			(DRD_IN  | DRT_STD | DRR_DEVICE)		// Request made to device,
#define OUT_INTERFACE		(DRD_OUT | DRT_STD | DRR_INTERFACE)		// Request made to interface,
#define IN_INTERFACE		(DRD_IN  | DRT_STD | DRR_INTERFACE)		// Request made to interface,
#define OUT_ENDPOINT		(DRD_OUT | DRT_STD | DRR_ENDPOINT)		// Request made to endpoint,
#define IN_ENDPOINT			(DRD_IN  | DRT_STD | DRR_ENDPOINT)		// Request made to endpoint,

#define OUT_CL_INTERFACE	(DRD_OUT | DRT_CLASS | DRR_INTERFACE)	// Request made to class interface,
#define IN_CL_INTERFACE		(DRD_IN  | DRT_CLASS | DRR_INTERFACE)	// Request made to class interface,

#define OUT_VR_INTERFACE	(DRD_OUT | DRT_VENDOR | DRR_INTERFACE)	// Request made to vendor interface,
#define IN_VR_INTERFACE		(DRD_IN  | DRT_VENDOR | DRR_INTERFACE)	// Request made to vendor interface,

/*
// Standard Request Codes
#define GET_STATUS				0x00		// Code for Get Status
#define CLEAR_FEATURE			0x01		// Code for Clear Feature
#define STD_REQ_RESERVE1		0x02		// reserved
#define SET_FEATURE				0x03		// Code for Set Feature
#define STD_REQ_RESERVE2		0x04		// reserved
#define SET_ADDRESS				0x05		// Code for Set Address
#define GET_DESCRIPTOR			0x06		// Code for Get Descriptor
#define SET_DESCRIPTOR			0x07		// Code for Set Descriptor(not used)
#define GET_CONFIGURATION		0x08		// Code for Get Configuration
#define SET_CONFIGURATION		0x09		// Code for Set Configuration
#define GET_INTERFACE			0x0A		// Code for Get Interface
#define SET_INTERFACE			0x0B		// Code for Set Interface
#define SYNCH_FRAME				0x0C		// Code for Synch Frame(not used)
*/

// Descriptor type (GET_DESCRIPTOR and SET_DESCRIPTOR)
#define DST_DEVICE				0x01		// Device Descriptor
#define DST_CONFIG				0x02		// Configuration Descriptor
#define DST_STRING				0x03		// String Descriptor
#define DST_INTERFACE			0x04		// Interface Descriptor
#define DST_ENDPOINT			0x05		// Endpoint Descriptor

// Define wValue bitmaps for Standard Feature Selectors
#define DEVICE_REMOTE_WAKEUP	0x01		// Remote wakeup feature(not used)
#define ENDPOINT_HALT			0x00		// Endpoint_Halt feature selector

//-----------------------------------------------------------------------------
// Definition of device and endpoint state
//-----------------------------------------------------------------------------

// Define device states
#define DEV_ATTACHED			0x00		// Device is in Attached State
#define DEV_POWERED				0x01		// Device is in Powered State
#define DEV_DEFAULT				0x02		// Device is in Default State
#define DEV_ADDRESS				0x03		// Device is in Addressed State
#define DEV_CONFIGURED			0x04		// Device is in Configured State
#define DEV_SUSPENDED			0x05		// Device is in Suspended State

// Define Endpoint States
#define EP_IDLE					0x00		// This signifies Endpoint Idle State
#define EP_HALT					0x01		// Endpoint Halt State (return stalls)
// for EP0
#define EP_TX					0x02		// Endpoint Transmit State
#define EP_RX					0x03		// Endpoint Receive State
// Endpoint Stall (send procedural stall next status phase)
#define EP_STALL				0x04

//! @defgroup std_request USB device standard requests decoding module
//! @{


//_____ M A C R O S ________________________________________________________


//_____ S T A N D A R D    D E F I N I T I O N S ___________________________

        // Device State
#define ATTACHED                          0
#define POWERED                           1
#define DEFAULT                           2
#define ADDRESSED                         3
#define CONFIGURED                        4
#define SUSPENDED                         5

#define USB_CONFIG_ATTRIBUTES_RESERVED    0x80
#define USB_CONFIG_BUSPOWERED            (USB_CONFIG_ATTRIBUTES_RESERVED | 0x00)
#define USB_CONFIG_SELFPOWERED           (USB_CONFIG_ATTRIBUTES_RESERVED | 0x40)
#define USB_CONFIG_REMOTEWAKEUP          (USB_CONFIG_ATTRIBUTES_RESERVED | 0x20)


//_____ D E C L A R A T I O N S ____________________________________________

  //! @brief Returns TRUE when device connected and correctly enumerated with a host.
  //! The device high-level application should test this before performing any applicative request.
#define Is_device_enumerated()            (usb_configuration_nb != 0)

  //! This function reads the SETUP request sent to the default control endpoint
  //! and calls the appropriate function. When exiting of the usb_read_request
  //! function, the device is ready to manage the next request.
  //!
  //! If the received request is not supported or a non-standard USB request, the function
  //! will call the custom decoding function in usb_specific_request module.
  //!
  //! @note List of supported requests:
  //! GET_DESCRIPTOR
  //! GET_CONFIGURATION
  //! SET_ADDRESS
  //! SET_CONFIGURATION
  //! CLEAR_FEATURE
  //! SET_FEATURE
  //! GET_STATUS
  //!
extern void usb_process_request(void);

extern volatile U8 usb_configuration_nb;

//! @}


#endif  // _USB_STANDARD_REQUEST_H_
