/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* This header file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

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
 */

#ifndef _USB_SPECIFIC_REQUEST_H_
#define _USB_SPECIFIC_REQUEST_H_


//_____ I N C L U D E S ____________________________________________________

#include "conf_usb.h"

#if USB_DEVICE_FEATURE == DISABLED
  #error usb_specific_request.h is #included although USB_DEVICE_FEATURE is disabled
#endif




//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

extern U8 usb_feature_report[3];
extern U8 usb_report[3];

extern U8 g_u8_report_rate;

extern volatile  U16	usb_interface_nb;
extern volatile  U8	usb_alternate_setting, usb_alternate_setting_out;
extern volatile  Bool  usb_alternate_setting_changed, usb_alternate_setting_out_changed;

typedef union {
	U32 frequency;
	U8 freq_bytes[4];
} S_freq;

extern S_freq current_freq;
extern Bool freq_changed;

//! @defgroup specific_request USB device specific requests
//! @{

//! @brief This function configures the endpoints of the device application.
//! This function is called when the set configuration request has been received.
//!
//! The core of this function should be correctly rewritten depending on the USB device
//! application characteristics (the USB device application has specific endpoint
//! configuration).
//!
extern void usb_user_endpoint_init(U8);

extern void usb_user_set_interface(U8 wIndex, U8 wValue);

//! This function is called by the standard USB read request function when
//! the USB request is not supported. This function returns TRUE when the
//! request is processed. This function returns FALSE if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard USB read request function.
//!
extern Bool usb_user_read_request(U8, U8);
extern Bool usb_user_get_descriptor(U8, U8);
extern Bool usb_user_DG8SAQ(U8, U8); // for processing DG8SAQ type of commands

// dg8saq EP0 hooks for the Mobo firmware
extern void dg8saqFunctionWrite(U8, U16, U16, U8 *, U8 );
extern U8 dg8saqFunctionSetup(U8, U16, U16, U8 *);

//! @}

//-----------------------------------------------------------------------------
// Definition of Class specific request for CDC
//-----------------------------------------------------------------------------

// CDC ACM class specifc requests
#define SEND_ENCAPSULATED_COMMAND	0x00
#define GET_ENCAPSULATED_RESPONSE	0x01
#define SET_LINE_CODING				0x20
#define GET_LINE_CODING				0x21
#define SET_CONTROL_LINE_STATE		0x22
#define SEND_BREAK					0x23


//! @brief This function manages reception of line coding parameters (baudrate...).
//!
void  cdc_get_line_coding(void);

//! @brief This function manages reception of line coding parameters (baudrate...).
//!
void  cdc_set_line_coding(void);

//! @brief This function manages the SET_CONTROL_LINE_LINE_STATE CDC request.
//!
//! @todo Manages here hardware flow control...
//!
void 	cdc_set_control_line_state (void);

// ____ T Y P E  D E F I N I T I O N _______________________________________

typedef struct
{
   U32 dwDTERate;
   U8 bCharFormat;
   U8 bParityType;
   U8 bDataBits;
}S_line_coding;

//! @}


#endif  // _USB_SPECIFIC_REQUEST_H_
