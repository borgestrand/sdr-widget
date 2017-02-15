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
#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20) // Use same buffer size in and out of SPDIF interface
	#define ADC_BUFFER_SIZE	(32*2*16) // Use 2^n style number, now: 512 stereo samples!
	#define DAC_BUFFER_SIZE (32*2*16) // Both: one half-buffer holds = ideal latency 11.6/10.67/5.8/5.33/2.9/2.67ms
#else
	#define ADC_BUFFER_SIZE	48*2*8 // 48 khz, stereo, 8 ms worth
	#define DAC_BUFFER_SIZE 48*2*16
#endif


// BSB 20131201 attempting improved playerstarted detection.
#define USB_BUFFER_TOGGLE_LIM 2		// DMA towards DAC I2S has toogled buffers too many times. 0 is ideal number
#define USB_BUFFER_TOGGLE_PARK 10	// The error is detected in sequential code

// Available digital audio sources, 3 and 4 only available in HW_GEN_DIN10 and ..20. Source 5 only available in HW_GEN_DIN20
#define MOBO_SRC_NONE		0
#define MOBO_SRC_UAC1		1
#define MOBO_SRC_UAC2		2
#define MOBO_SRC_SPDIF		3
#define MOBO_SRC_TOS2		4
#define MOBO_SRC_TOS1		5


// Front led colors for RGB LEDs
#define FLED_RED			1
#define FLED_GREEN			2
#define FLED_YELLOW			3
#define FLED_BLUE			4
#define FLED_PURPLE			5
#define FLED_CYAN			6
#define FLED_WHITE			7
#define FLED_DARK			0

// USB channels
#define USB_CH_NONE			0
#define USB_CH_A			1
#define USB_CH_B			2
#define USB_CH_NOSWAP		0		// NO USB channel swapping happening
#define USB_CH_SWAPDET		1		// Need for channel swap detected
#define USB_CH_SWAPACK		2		// Channel swap detect acknowledged by uac?_device_audio_task

// Frequency definitions, move and change to make compatible with USB system!
#define	FREQ_TIMEOUT		0x00
#define	FREQ_32				32000
#define	FREQ_44				44100
#define	FREQ_48				48000
#define	FREQ_88				88200
#define	FREQ_96				96000
#define	FREQ_176			176400
#define	FREQ_192			192000

// Values for silence (32-bit)
#define SILENCE_USB_LIMIT	12000 				// We're counting USB packets. UAC2: 250us, UAC1: 1ms. Value of 12000 means 3s
#define SILENCE_USB_INIT	0
#define USB_IS_SILENT() (silence_USB >= SILENCE_USB_LIMIT)


//extern const gpio_map_t SSC_GPIO_MAP;
//extern const pdca_channel_options_t PDCA_OPTIONS;
//extern const pdca_channel_options_t SPK_PDCA_OPTIONS;

// Global buffer variables
extern volatile U32 audio_buffer_0[ADC_BUFFER_SIZE];
extern volatile U32 audio_buffer_1[ADC_BUFFER_SIZE];
extern volatile U32 spk_buffer_0[DAC_BUFFER_SIZE];
extern volatile U32 spk_buffer_1[DAC_BUFFER_SIZE];
extern volatile avr32_ssc_t *ssc;
extern volatile int ADC_buf_DMA_write; // Written by interrupt handler, initiated by sequential code
extern volatile int DAC_buf_DMA_read; // Written by interrupt handler, initiated by sequential code
extern volatile int ADC_buf_USB_IN; // Written by sequential code
extern volatile int DAC_buf_USB_OUT; // Written by sequential code
extern volatile avr32_pdca_channel_t *pdca_channel;
extern volatile avr32_pdca_channel_t *spk_pdca_channel;
#if ((defined HW_GEN_DIN10) || (defined HW_GEN_DIN20))
	extern volatile U16 ADC_num_remaining;
#endif

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
