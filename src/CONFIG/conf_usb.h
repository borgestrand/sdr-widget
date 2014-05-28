/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* This header file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief USB configuration file.
 *
 * This file contains the possible external configuration of the USB.
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
 * Modified by Alex Lee 20 Feb 2010
 * To enumerate as a USB composite device:
 * HID (generic HID interface, compatible with Jan Axelson's generichid.exe test programs
 * DG8SAQ (libusb API compatible interface for implementing DG8SAQ EP0 type of interface)
 * Audio (Audio Class v1 and Audio Class V2.  Tweaked for
 * 		compatibility when running at HIGH speed USB.)
 * For SDR-Widget and SDR-Widget-lite, custom boards based on the AT32UC3A3256
 *
 * See http://code.google.com/p/sdr-widget/
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


#ifndef _CONF_USB_H_
#define _CONF_USB_H_

#include "compiler.h"
#include "board.h"
#include "print_funcs.h"
#include "usb_ids.h"


//! @defgroup usb_general_conf USB application configuration
//!
//! @{

  // _________________ USB MODE CONFIGURATION ____________________________
  //
  //! @defgroup USB_op_mode USB operating modes configuration
  //! Defines to enable device or host USB operating modes
  //! supported by the application
  //! @{

    //! @brief ENABLE to activate the host software framework support
    //!
    //! Possible values ENABLED or DISABLED
#define USB_HOST_FEATURE            DISABLED
    //! @brief ENABLE to activate the device software framework support
    //!
    //! Possible values ENABLED or DISABLED
#define USB_DEVICE_FEATURE          ENABLED

#define USB_HIGH_SPEED_SUPPORT         ENABLED

  //! @}


  // _________________ HOST MODE CONFIGURATION ____________________________
  //
  //! @defgroup USB_host_mode_cfg USB host operating mode configuration
  //!
  //! @{



  // _________________ DEVICE MODE CONFIGURATION __________________________
  //
  //! @defgroup USB_device_mode_cfg USB device operating mode configuration
  //!
  //! @{

#if USB_DEVICE_FEATURE == ENABLED

#define COMPOSITE_DEVICE

// UAC1 endpoints
#define UAC1_EP_HID_TX			1
#define UAC1_EP_HID_RX			2
#define UAC1_EP_AUDIO_OUT		4
#define UAC1_EP_AUDIO_IN		5
#define UAC1_EP_AUDIO_OUT_FB	3

// UAC2 endpoints
#define UAC2_EP_AUDIO_IN		3
#define UAC2_EP_AUDIO_OUT		2
#define UAC2_EP_AUDIO_OUT_FB	1
#define UAC2_EP_HID_TX			4
#define UAC2_EP_HID_RX			5

// HPSDR endpoints
#define HPSDR_EP_RF_IN			4
#define HPSDR_EP_IQ_IN			6
#define HPSDR_EP_IQ_OUT			2

// temporary resolution
// #define EP_AUDIO_IN		UAC2_EP_AUDIO_IN
// #define EP_AUDIO_OUT	UAC2_EP_AUDIO_OUT
// #define EP_AUDIO_OUT_FB	UAC2_EP_AUDIO_OUT_FB
// #define EP_HID_TX		UAC2_EP_HID_TX
// #define EP_HID_RX		UAC2_EP_HID_RX

//! @defgroup device_cst_actions USB device custom actions
//!
//! @{
// Write here the action to associate with each USB event.
// Be careful not to waste time in order not to disturb the functions.
#define Usb_sof_action()
#define Usb_wake_up_action()
#define Usb_resume_action()
#define Usb_suspend_action()
#define Usb_reset_action()
#define Usb_vbus_on_action()
#define Usb_vbus_off_action()
#define Usb_set_configuration_action()
//! @}

#endif  // USB_DEVICE_FEATURE == ENABLED

//! @}


//! USB interrupt priority level
#define USB_INT_LEVEL                   AVR32_INTC_INT0

//! Debug trace macro
#define LOG_STR(str)                    //print_dbg(str)

//! @defgroup usb_stream_control USB stream control parameters
//! Defines the way the USB stream control will operate. The USB Stream Control embeds a mechanism
//! that ensures a good audio playback by keeping synchronized both Host and Device, even if their
//! sampling frequency are not strictly equivalent.
//! @{

//! Size of a buffer (in bytes) used in the USB stream FIFO. It shall be equivalent to the pipe/endpoint
//! from which the stream comes to.
#define USB_STREAM_BUFFER_SIZE        100   // Size in bytes.

//! Number of buffers used in the USB stream FIFO.
#define USB_STREAM_BUFFER_NUMBER        8   // Unit is in number of buffers. Must be a 2-power number.

//! Maximum gap (in number of buffers) between the stream reader and the stream writer, in which the FIFO
//! operates without re-synchronization.
#define USB_STREAM_IDLE_BUFFER_NUMBER   2   // Unit is in number of buffers.

//! Max sampling frequencies excursion (given in per-thousandth) that the USB Stream Control FIFO
//! is supposed to softly correct.
#define USB_STREAM_MAX_EXCURSION       100  // Unit is in per-thousandth (�/oo)
 
 //! @}
 
 

//! @}


#endif  // _CONF_USB_H_
