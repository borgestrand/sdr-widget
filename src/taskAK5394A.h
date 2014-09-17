/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * taskAK5394A.h
 *
 *  Created on: Feb 16, 2010
 *  Refactored on: Feb 26, 2011
 *      Author: Alex
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

#ifndef TASKAK5394A_H_
#define TASKAK5394A_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define PDCA_CHANNEL_SSC_RX	   0	// highest priority of 8 channels
#define PDCA_CHANNEL_SSC_TX	   1
#define AUDIO_BUFFER_SIZE	(48*2*8) // 48 khz, stereo, 8 ms worth
#define SPK_BUFFER_SIZE 	(48*2*16)

// BSB 20131201 attempting improved playerstarted detection.
#define USB_BUFFER_TOGGLE_LIM 2		// DMA towards DAC I2S has toogled buffers too many times. 0 is ideal number
#define USB_BUFFER_TOGGLE_PARK 10	// The error is detected in sequential code


//extern const gpio_map_t SSC_GPIO_MAP;
//extern const pdca_channel_options_t PDCA_OPTIONS;
//extern const pdca_channel_options_t SPK_PDCA_OPTIONS;

extern volatile U32 audio_buffer_0[AUDIO_BUFFER_SIZE];
extern volatile U32 audio_buffer_1[AUDIO_BUFFER_SIZE];
extern volatile U32 spk_buffer_0[SPK_BUFFER_SIZE];
extern volatile U32 spk_buffer_1[SPK_BUFFER_SIZE];
extern volatile avr32_ssc_t *ssc;
extern volatile int audio_buffer_in;
extern volatile int spk_buffer_out;
extern volatile U32 spk_usb_heart_beat, old_spk_usb_heart_beat;
extern volatile U32 spk_usb_sample_counter, old_spk_usb_sample_counter;
extern xSemaphoreHandle mutexSpkUSB;

// BSB 20131201 attempting improved playerstarted detection
extern volatile S32 usb_buffer_toggle;

// BSB 20140917 attempting to help uacX_device_audio_task.c synchronize to DMA
extern volatile U8 audio_OUT_alive;
extern volatile U8 audio_OUT_must_sync;

void AK5394A_pdca_disable(void);
void AK5394A_pdca_enable(void);
void AK5394A_task_init(Bool uac2);

#endif /* TASKAK5394A_H_ */
