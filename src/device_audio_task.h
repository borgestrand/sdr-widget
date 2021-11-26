/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* This header file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Management of the USB device mouse Audio task.
 *
 * This file manages the USB device mouse Audio task.
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
 * Modified by Alex Lee and SDR-Widget team for the sdr-widget project.
 * See http://code.google.com/p/sdr-widget/
 * Copyright under GNU General Public License v2
 */

#ifndef _DEVICE_AUDIO_TASK_H_
#define _DEVICE_AUDIO_TASK_H_


//_____ I N C L U D E S ____________________________________________________

#include "conf_usb.h"
#include <stdint.h>

// To access semaphore
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "gpio.h"

// To access SPDIF RX status
#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)
#include "wm8805.h"
#endif

#if (defined HW_GEN_RXMOD)
#include "wm8804.h"
#include "pcm5142.h"
#endif


#if USB_DEVICE_FEATURE == DISABLED
  #error device_audio_task.h is #included although USB_DEVICE_FEATURE is disabled
#endif


//_____ D E F I N I T I O N S ______________________________________________

#define FB_NOMINAL_OFFSET 0 //512						// BSB 20131101 Offset used to test provoked feedback system

// FB_rate: state machine's feedback rate, communicated to Host over feedback EP
// FB_rate_initial: The initial setting, including any special Linux settings, NOT offset. Used to re-init state machine
// FB_rate_nominal: The best setting if Host and DAC are fully synced. Offset by FB_NOMINAL_OFFSET to simulate a stupid or dead Host feedback

extern volatile S32 FB_rate, FB_rate_initial, FB_rate_nominal; // BSB 20131031 FB_rate_initial and FB_rate_nominal added and changed to S32
extern volatile Bool mute, spk_mute;
extern volatile uint8_t usb_spk_mute; // This variable is written to by usb subsystem and heeded in playback

extern S16 spk_vol_usb_L, spk_vol_usb_R;			// BSB 20160320 added stereo volume control
extern S32 spk_vol_mult_L, spk_vol_mult_R;

extern volatile uint8_t input_select;				// BSB 20150501 global variable for input selector


// RXMODFIX Global variables for tuning scanning algorithm
extern volatile uint8_t wm8804_LINK_MAX_ATTEMPTS;
extern volatile uint8_t wm8804_LINK_DETECTS_OK;
extern volatile uint8_t wm8804_TRANS_ERR_FAILURE;




#if (defined HW_GEN_DIN20) || (defined HW_GEN_RXMOD)
extern volatile uint8_t usb_ch;						// Front or rear USB channel
extern volatile uint8_t usb_ch_swap;				// Front or rear USB channel
#endif

#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20) || (defined HW_GEN_RXMOD)
extern volatile xSemaphoreHandle input_select_semphr; 	// BSB 20150626 audio channel selection semaphore
extern volatile spdif_rx_status_t spdif_rx_status;
#endif

#if (defined HW_GEN_RXMOD)
extern volatile xSemaphoreHandle I2C_busy; 			// One semaphore covers entire I2C message, not just function calls. Not yet ported to all I2C callers
#endif


//_____ M A C R O S ________________________________________________________


//_____ D E C L A R A T I O N S ____________________________________________


#endif  // _DEVICE_AUDIO_TASK_H_
