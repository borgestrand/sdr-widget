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
#include "hpsdr_usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "usart.h"
#include "pm.h"
#include "Mobo_config.h"
#include "usb_audio.h"
#include "device_audio_task.h"
#include "hpsdr_device_audio_task.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________


//_____ P R I V A T E   D E C L A R A T I O N S ____________________________

// U8 usb_feature_report[3];
// U8 usb_report[3];

// U8 g_u8_report_rate=0;

// S_line_coding   line_coding;

// S_freq current_freq;
// Bool freq_changed = FALSE;

static U8    wValue_msb;
static U8    wValue_lsb;
static U16   wIndex;
static U16   wLength;

//_____ D E C L A R A T I O N S ____________________________________________


//! @brief This function configures the endpoints of the device application.
//! This function is called when the set configuration request has been received.
//!
void hpsdr_user_endpoint_init(U8 conf_nb)
{
	if( Is_usb_full_speed_mode() ) {
		(void)Usb_configure_endpoint(HPSDR_EP_RF_IN, EP_ATTRIBUTES_1, DIRECTION_IN, EP_SIZE_1_FS, DOUBLE_BANK, 0);
		(void)Usb_configure_endpoint(HPSDR_EP_IQ_IN, EP_ATTRIBUTES_2, DIRECTION_IN, EP_SIZE_2_FS, DOUBLE_BANK, 0);
		(void)Usb_configure_endpoint(HPSDR_EP_IQ_OUT, EP_ATTRIBUTES_3, DIRECTION_OUT, EP_SIZE_3_FS, DOUBLE_BANK, 0);
	} else {
		(void)Usb_configure_endpoint(HPSDR_EP_RF_IN, EP_ATTRIBUTES_1, DIRECTION_IN, EP_SIZE_1_HS, DOUBLE_BANK, 0);
		(void)Usb_configure_endpoint(HPSDR_EP_IQ_IN, EP_ATTRIBUTES_2, DIRECTION_IN, EP_SIZE_2_HS, DOUBLE_BANK, 0);
		(void)Usb_configure_endpoint(HPSDR_EP_IQ_OUT, EP_ATTRIBUTES_3, DIRECTION_OUT, EP_SIZE_3_HS, DOUBLE_BANK, 0);
	}
}

//! @brief This function handles usb_set_interface side effects.
//! This function is called from usb_set_interface in usb_standard_request
//! in case there are side effects of the interface change to be handled.
void hpsdr_user_set_interface(U8 wIndex, U8 wValue) {
}

//! This function is called by the standard USB read request function when
//! the USB request is not supported. This function returns TRUE when the
//! request is processed. This function returns FALSE if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard USB read request function.
//!
Bool hpsdr_user_read_request(U8 type, U8 request)
{
   // Read wValue
   wValue_lsb = Usb_read_endpoint_data(EP_CONTROL, 8);
   wValue_msb = Usb_read_endpoint_data(EP_CONTROL, 8);
   wIndex = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
   wLength = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));

   return FALSE;  // No supported request
}

#endif  // USB_DEVICE_FEATURE == ENABLED
