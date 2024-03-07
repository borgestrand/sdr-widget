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
// Keep buffer sizes belov 2^14
/* Nominal values are (8*2*24) and (32*2*24)
Long buffers may take up too much RAM. And clearing and moving their contents take a long time.
Short buffers give less system latency and poorer synch state machine performance
*/
#define ADC_BUFFER_SIZE 1024	// Must be divisible by 4
#define DAC_BUFFER_UNI 1536*2 		// Was: 1536*2 // (32*2*24) * 1 // = 1536

#define SPK_CACHE_MAX_SAMPLES 120	// Maximum number of stereo samples in two package of 250�s (nominally 48 at 192ksps). That way we can miss one. As global 120 is OK. As local it refused to run above 60

#define IS_SILENT		0x00040000 // Compare abs(sample) to 4 LSBs at 16-bit audio, 1024 LSBs at 24-bit audio

#if (defined HW_GEN_SPRX) || (defined HW_GEN_FMADC) // ADC must be at least 4 times as fast as DAC in order to monitor SPDIF buffering
	// Set up spdif receive timer to fire approximately once every 250�s (UAC2) or 1ms (UAC1) during SPDIF packet processing
	// MCU has "Two Three-Channel 16-bit Timer/Counter (TC)" Each timer has three channels
	#define SPDIF_TC_DEVICE		AVR32_TC1	// Using TC1 where we have CLK0 available on PA05
	#define SPDIF_TC_CHANNEL	0			// Timer counter -channel-
#endif


// BSB 20131201 attempting improved playerstarted detection.
#define USB_BUFFER_TOGGLE_LIM 4		// Changed from 2 to 4 after hassle with Sue's phone. DMA towards DAC I2S has toogled buffers too many times. 0 is ideal number
#define USB_BUFFER_TOGGLE_PARK 10	// The error is detected in sequential code

// Available digital audio sources, 3 and 4 only available in HW_GEN_DIN10 and ..20. Source 5 only available in HW_GEN_DIN20n and HW_GEN_SPRX
#define MOBO_SRC_NONE		0
#define MOBO_SRC_UAC2		2
#define MOBO_SRC_SPDIF0		3
#define MOBO_SRC_TOSLINK1	4
#define MOBO_SRC_TOSLINK0	5
#define MOBO_SRC_SPDIF1		6		// Future auxilliary SPDIF channel on computer header or for HDMI FIX: propagate throughout code!
#define MOBO_SRC_HIGH		5		// Highest source indicator for SPDIF/TOSLINK RX FIX: increase to 6 with aux SPDIF
#define MOBO_SRC_LOW		3		// Lowest source indicator for SPDIF/TOSLINK RX
#define MOBO_SRC_MUXED		0xFE	// Whatever channel is selected by passive MUX
#define MOBO_SRC_INVALID	0xFF


// Front led colors for RGB LEDs
#define FLED_RED			1
#define FLED_GREEN			2
#define FLED_YELLOW			3
#define FLED_BLUE			4
#define FLED_PURPLE			5
#define FLED_CYAN			6
#define FLED_WHITE			7
#define FLED_DARK			0
#define FLED_NO_CHG			9
#define FLED_SCANNING		FLED_WHITE	// While scanning for an input, should there be a default light? Implemented only on HW_GEN_SPRX

// USB channels
#define USB_CH_NONE			0		// No USB port has been detected
#define USB_CH_DEACTIVATE	1		// Actively disconnecting USB mux for debug purposes
#define USB_CH_A			2		// Name used in HW_GEN_DIN20 for front USB-C plug
#define USB_CH_B			3
#define USB_CH_C			4		// Name used in HW_GEN_SPRX for front USB-C plug
#define USB_CH_NOSWAP		0		// NO USB channel swapping happening
#define USB_CH_SWAPDET		1		// Need for channel swap detected
#define USB_CH_SWAPACK		2		// Channel swap detect acknowledged by uac?_device_audio_task

// Frequency definitions, move and change to make compatible with USB system!
#define	FREQ_TIMEOUT		0x00
#define FREQ_INVALID		1
#define FREQ_RXNATIVE		2		// Use recovered MCLK of SPDIF receiver. Only used as parameter to mobo_xo_select()
#define FREQ_PLLMISS		3
#define FREQ_NOCHANGE		4
#define	FREQ_32				32000
#define	FREQ_44				44100
#define	FREQ_48				48000
#define	FREQ_88				88200
#define	FREQ_96				96000
#define	FREQ_176			176400
#define	FREQ_192			192000
#define DAC_MUST_CLEAR		1		// Immediately clear the content of outgoing DAC buffers
#define DAC_CLEARED			2		// Outgoing DAC buffers are cleared, don't write to DAC buffers
#define DAC_READY			3		// Outgoing DAC buffers are ready to be written to
#define INIT_ADC_I2S		-1		// Must initialize the buffer pointer used for the I2S toward DAC
#define INIT_ADC_I2S_st2	-2		// Must initialize the buffer pointer used for the I2S toward DAC, init stage 2
#define INIT_ADC_USB		-3		// Must initialize the buffer pointer used for the I2S toward USB
#define INIT_ADC_USB_st2	-4		// Must initialize the buffer pointer used for the I2S toward USB

#ifdef FEATURE_ADC_EXPERIMENTAL
	#define I2S_CONSUMER_NONE	0b00000000		// None
	#define I2S_CONSUMER_USB	0b00000001		// USB is consuming I2S input data
	#define I2S_CONSUMER_DAC	0b00000010		// DAC output is consuming I2S input data
#endif


// Values for silence (32-bit)
#define SILENCE_USB_LIMIT	12000 				// We're counting USB packets. UAC2: 250us, UAC1: 1ms. Value of 12000 means 3s
#define SILENCE_USB_INIT	0
#define USB_IS_SILENT() (silence_USB >= SILENCE_USB_LIMIT)


//extern const gpio_map_t SSC_GPIO_MAP;
//extern const pdca_channel_options_t PDCA_OPTIONS;
//extern const pdca_channel_options_t SPK_PDCA_OPTIONS;

// Global buffer variables
extern volatile S32 audio_buffer[ADC_BUFFER_SIZE];
extern volatile S32 spk_buffer[DAC_BUFFER_UNI];
extern volatile bool must_init_spk_index;
extern volatile S32 cache_L[SPK_CACHE_MAX_SAMPLES];	// This shouldn't need to be global, it only exists in uac2_dat2.c and whatever it calls
extern volatile S32 cache_R[SPK_CACHE_MAX_SAMPLES];

extern volatile avr32_ssc_t *ssc;

// ���� old buffer ids ripe for renaming or removal
extern volatile int ADC_buf_I2S_IN; 	// Written by sequential code, handles only data coming in from I2S interface (ADC or SPDIF rx)
extern volatile int ADC_buf_USB_IN;		// Written by sequential code, handles only data IN-to USB host
extern volatile int DAC_buf_OUT;		// Written by sequential code
extern volatile avr32_pdca_channel_t *pdca_channel;
extern volatile avr32_pdca_channel_t *spk_pdca_channel;
extern volatile int dac_must_clear;	// uacX_device_audio_task.c must clear the content of outgoing DAC buffers

#ifdef HW_GEN_SPRX
	// SPDIF timer/counter records DMA status - global registers move data from interrupt handler
	extern volatile U32 timer_captured_num_remaining;
#endif


#ifdef FEATURE_ADC_EXPERIMENTAL
	extern volatile U8 I2S_consumer;		// Which consumer is subscribing to I2S data? 
#endif


extern volatile U32 spk_usb_heart_beat, old_spk_usb_heart_beat;
extern volatile U32 spk_usb_sample_counter, old_spk_usb_sample_counter;
extern xSemaphoreHandle mutexSpkUSB;

// BSB 20131201 attempting improved playerstarted detection 
extern volatile S32 usb_buffer_toggle;

// BSB 20140917 attempting to help uacX_device_audio_task.c synchronize to DMA
// extern volatile U8 audio_OUT_alive;

// BSB 20170324 SPDIF buffer processor detects silence
extern volatile U8 dig_in_silence;

void AK5394A_task_init(Bool uac2);

// New code polls DAC LRCK
void AK5394A_pdca_tx_enable(U32 frequency);

// New code polls ADC LRCK
void AK5394A_pdca_rx_enable(U32 frequency);

#endif /* TASKAK5394A_H_ */
