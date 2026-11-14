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
#include "stdint.h"

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
// #define IS_SILENT		0x00010000 // Is: 1LSBs at 16-bit level. Was: 0x00040000 Approximately: compare (si_score_high + abs(sample_L >> 8) + abs(sample_R) >> 8) to 4 LSBs at 16-bit audio, 1024 LSBs at 24-bit audio

#if (defined HW_GEN_SPRX) || (defined HW_GEN_FMADC) // ADC must be at least 4 times as fast as DAC in order to monitor SPDIF buffering
	// Set up spdif receive timer to fire approximately once every 250�s (UAC2) or 1ms (UAC1) during SPDIF packet processing
	// MCU has "Two Three-Channel 16-bit Timer/Counter (TC)" Each timer has three channels
	#define SPDIF_TC_DEVICE		AVR32_TC1	// Using TC1 where we have CLK0 available on PA05
	#define SPDIF_TC_CHANNEL	0			// Timer counter -channel-
#endif


// BSB 20131201 attempting improved playerstarted detection.
#define USB_BUFFER_TOGGLE_LIM 4		// Changed from 2 to 4 after hassle with Sue's phone. DMA towards DAC I2S has toogled buffers too many times. 0 is ideal number
#define USB_BUFFER_TOGGLE_PARK 10	// The error is detected in sequential code

// 20261011: MOBO_SRC_ source definitions moved to Mobo_config.h


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
#define FLED_SCANNING		FLED_WHITE	// While idle and scanning for an input, should there be a default light? Implemented only on HW_GEN_SPRX

// USB channels
#define USB_CH_NONE			0		// No USB port has been detected
#define USB_CH_DEACTIVATE	1		// Actively disconnecting USB mux for debug purposes
#define USB_CH_B			3
#define USB_CH_C			4		// Name used in HW_GEN_SPRX for front USB-C plug
#define USB_CH_NOSWAP		0		// NO USB channel swapping happening
#define USB_CH_SWAPDET		1		// Need for channel swap detected
#define USB_CH_SWAPACK		2		// Channel swap detect acknowledged by uac?_device_audio_task

// Frequency definitions, move and change to make compatible with USB system!
#define	FREQ_TIMEOUT		192001	// All return values, from 32000 to 192002 are way above the limits defined in mobo_srd_asm2()
#define FREQ_INVALID		192002
#define FREQ_RXNATIVE_EN	2		// Use recovered MCLK of SPDIF receiver. Only used as parameter to mobo_xo_select()
#define FREQ_RXNATIVE_DIS	3		// Revert to XO. Only used as parameter to mobo_xo_select()
#define FREQ_PLLMISS		4
#define FREQ_NOCHANGE		5
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
	#define I2S_CONSUMER_NONE	0b00000000	// None
	#define I2S_CONSUMER_USB	0b00000001	// USB is consuming I2S input data
	#define I2S_CONSUMER_DAC	0b00000010	// DAC output is consuming I2S input data
#endif


// Values for silence (32-bit)
#define SILENCE_USB_LIMIT		12000 		// We're counting USB packets. UAC2: 250us, UAC1: 1ms. Value of 12000 means 3s
#define SILENCE_USB_INIT		0
#define USB_IS_SILENT() (silence_USB >= SILENCE_USB_LIMIT)

#define SILENCE_SPDIF_LIMIT		12000 		// We're counting SPDIF packets of 250us. Value of 12000 means 3s
#define SILENCE_SPDIF_SCANNING	800 		// We're counting SPDIF packets of 250us. Value of 800 means 200ms. Derived from WM8804_SILENCE_LINKING
#define SILENCE_SPDIF_INIT		0
#define SPDIF_IS_SILENT() (spdif_rx_status.silence_SPDIF >= SILENCE_SPDIF_LIMIT)


//extern const gpio_map_t SSC_GPIO_MAP;
//extern const pdca_channel_options_t PDCA_OPTIONS;
//extern const pdca_channel_options_t SPK_PDCA_OPTIONS;

// Global buffer variables
extern volatile S32 audio_buffer[ADC_BUFFER_SIZE];
extern volatile S32 spk_buffer[DAC_BUFFER_UNI];
extern volatile bool must_init_spk_index;
extern volatile bool must_init_xo;
extern volatile S32 samples_per_package_min;
extern volatile S32 samples_per_package_max;
// extern volatile S32 cache_L[SPK_CACHE_MAX_SAMPLES];	// This shouldn't need to be global, it only exists in uac2_dat2.c and whatever it calls
// extern volatile S32 cache_R[SPK_CACHE_MAX_SAMPLES];
extern volatile S32 cache_unified[2*SPK_CACHE_MAX_SAMPLES];

// 20261021: RAM copy of one EP_AUDIO_OUT bank (max 512 bytes, Table 27-1).
// 20261029: filled by the USBB DMA. Cleared in mobo_clear_dac_channel() along with the other audio buffers
#define EP_OUT_RAM_BYTES	512
#ifdef FEATURE_EP_OUT_HSBRAM
	// 20261114: ep_out_ram at the start of HSB SRAM bank 0 (INTRAM0, its own bus matrix slave), so the USB DMA never meets
	// the I2S DMA at the CPU SRAM that holds spk_buffer. Nothing else is placed there. The HRAMC0 clock is enabled in main()
	extern U64 * const ep_out_ram;
#else
	extern U64 ep_out_ram[EP_OUT_RAM_BYTES / 8];
#endif

// 20261029: USB OUT reception by DMA (method 2, one packet per buffer so far). The USBB DMA channel of EP_AUDIO_OUT
// copies each bank into ep_out_ram and frees the bank (needs UECFGn.AUTOSW). The DMA-end interrupt sets ep_out_dma_flag
// and the length, the sequential code parses ep_out_ram and re-arms with ep_out_dma_arm(). Functions in
// uac2_usb_specific_request.c. First music 20261101. The earlier CPU-copy path (RXOUTI interrupt, flag, FIFO reads)
// is commented out below and in the files that used it; it was last present in commit fdb8ef6 (20261101)
extern volatile bool ep_out_dma_flag;		// Set by ep_out_dma_isr(), cleared by the consumer
extern volatile U16 ep_out_dma_len;			// Bytes in ep_out_ram
extern volatile U32 ep_out_dma_count;		// Debug: DMA-end interrupts
extern volatile U32 ep_out_dma_status;		// Debug: last UDDMAn STATUS
extern volatile U8 ep_out_dma_nbusybk;		// Debug: NBUSYBK right after DMA end. 0 when the DMA freed the bank
extern volatile U8 ep_out_dma_fifocon;		// Debug: FIFOCON right after DMA end
extern volatile U8 ep_out_dma_rxouti;		// Debug: RXOUTI right after DMA end (never acked by software, harmless)
#ifdef FEATURE_PRIME_CHECK
// 20261114: all debug tools of the I2S zero-word hunt (Z, O, E, H, !) are built in with FEATURE_PRIME_CHECK only.
// Without it, their commands reply '-'
// 20261105: debug 'Z' sets this. The USB audio task then writes 0 into L and R of one frame of spk_buffer and clears it,
// to show a known single-sample error on the scope
extern volatile bool spk_zero_frame;
// 20261105: debug 'O' steps through the prime analysis modes (FEATURE_PRIME_CHECK in uac2_device_audio_task.c). 0 at boot.
// 20261113: four modes, one bit per feature. 'O' steps 0 -> 1 -> 2 -> 3 -> 0 and prints the new mode
//   PRIME_MODE_CHECK: the USB prime check, as before
//   PRIME_MODE_PULSE: known I2S pulse width, USB playback only. The parse overwrites L and R of every USB frame with a
//   fixed bit pattern, right after the prime check, chosen by sample rate so that SDATA pulses are about 0.33us wide at
//   all rates: 0x55555555 (1 bit) at 44.1/48k, 0x33333333 (2 bits) at 88.2/96k, 0x0F0F0F0F (4 bits) at 176.4/192k.
//   A zero word shows on the scope as SDATA low for 32 bit clocks. Debug 'Z' still zeros its frame. I2S metadata, if
//   built in, still replaces bits 7:0
// 20261114: debug 'H' turns the bus matrix priority for the I2S DMA on and off (hmatrix_i2s_priority() in taskAK5394A.c).
// On: every bus matrix slave uses fixed priority with the PDCA highest, and the USB DMA re-arbitrates on every access
// (ULBT single). Off: the values found before the first 'H'. Off at boot
extern volatile bool hmatrix_i2s_first;
void hmatrix_i2s_priority(bool on);
#define PRIME_MODE_CHECK	0x01
#define PRIME_MODE_PULSE	0x02
extern volatile U8 prime_mode;
	// 20261106: log of prime breaks, written by the USB audio task, printed and emptied by debug 'E'. The log fills up and
	// then keeps its contents until printed. 'E' prints the whole log. Unused words hold DEADBEEF, at boot and after each 'E'.
	// One record per break ('<' and '>' alike), PRIME_LOG_WORDS words, all hex in the 'E' dump:
	// [0] FFFFFFFF marker. It is never a sample: 24-bit samples have bits 7:0 = 0, 16-bit samples bits 15:0 = 0
	// [1] break character ('<' or '>') in bits 31:24, frame index in the packet in bits 23:16, packet length in bytes in bits 15:0
	// [2] previous packet's length in bytes in bits 31:16, packets since the previous break in bits 15:0 (saturates at FFFF)
	// [3] [4] previous frame L, R   [5] [6] the breaking frame L, R   [7] [8] next frame L, R (may be in the next packet)
	// Samples are left aligned in 32 bits, as assembled from USB
	#define PRIME_LOG_EVENTS	32
	#define PRIME_LOG_WORDS		9
	extern volatile U32 prime_log[PRIME_LOG_EVENTS * PRIME_LOG_WORDS];
	extern volatile U8 prime_log_count;			// Complete records in prime_log
	extern volatile bool prime_log_frozen;		// Set by 'E' while printing: no new records are started
#endif
void ep_out_dma_arm(void);
void ep_out_dma_stop(void);
void ep_out_dma_isr(void);
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

/* 20261101: legacy RXOUTI-interrupt flag of the CPU-copy path, last present in commit fdb8ef6 (20261101)
extern volatile bool usb_audio_out_received_flag;
#define isr_Is_usb_out_received(ep)          (usb_audio_out_received_flag)
#define clear_isr_Is_usb_out_received_flag() (usb_audio_out_received_flag = FALSE)
*/

// 20261003: an endpoint reset (SET_INTERFACE, CLEAR_FEATURE HALT) resets the endpoint registers and banks.
// 20261029: it also leaves the DMA channel in an unknown state. Stop it, drop any stale flag and re-arm into ep_out_ram,
// or EP_AUDIO_OUT is never read again
#define EP_AUDIO_OUT_rearm_after_reset(ep) do { \
	if ((ep) == UAC2_EP_AUDIO_OUT) { \
		ep_out_dma_stop(); \
		ep_out_dma_flag = FALSE; \
		ep_out_dma_arm(); \
	} \
} while (0)

// 20260922: increments once per usb_general_interrupt_non_naked() firing on EP_AUDIO_OUT
// reception (usb_task.c) - independent proof the ISR is actually running, separate from
// whether the PA22 toggle is visibly working. Read over UART with the 'I' command in
// device_mouse_hid_task.c.
extern volatile uint32_t EP_AUDIO_OUT_isr_counter;

// BSB 20131201 attempting improved playerstarted detection 
extern volatile S32 usb_buffer_toggle;

void AK5394A_task_init(Bool uac2);

// New code polls DAC LRCK
void AK5394A_pdca_tx_enable(U32 frequency);

// New code polls ADC LRCK
void AK5394A_pdca_rx_enable(U32 frequency);

#endif /* TASKAK5394A_H_ */
