/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* uac2_device_audio_task_V2.c - 20261126: UAC2 audio task for SPRX, slimmed down for 352.8k / 384k.
 *
 * Included by uac2_device_audio_task.c with FEATURE_UAC2_TASK_V2. uac2_device_audio_task_Legacy.c (commit 35dc29d
 * and later) is the reference for everything left out here.
 *
 * Copyright under GNU General Public License v2, as the file it replaces. Parts from the Atmel AVR32 Software
 * Framework (Copyright (c) 2009 Atmel Corporation) and the SDR-Widget team, see the Legacy file for the notices.
 *
 * WHY
 * Legacy plays 32/192 at 49.152 MHz with PA22 high for about 180us per 250us buffer (48 frames), about 180
 * cycles per frame: the parse into cache_unified, then a second loop into spk_buffer. 352.8k / 384k doubles the
 * frames per buffer (96 at 384k), so at 49.152 MHz the budget is 12288 cycles per buffer, 128 cycles per frame
 * with nothing else running. V2 aims at 15-20 cycles per frame at 32 bit.
 * 20261126 bench, V2 PA22 per buffer: 32/384 at 66 MHz 39us (about 27 cycles per frame, all included),
 * 16/44.1 at 45.16 MHz 6.8us (11 frames, mostly fixed cost). Locked to MCLK ('m'), 32/384 at 49.152 MHz plays
 * clean with PA22 up to 69us
 *
 * SCOPE: bit-perfect USB playback, feedback and the slow (buffered) debug UART. Nothing else, until the speed is
 * there. Then features come back one at a time, each with its PA22 cost measured.
 * Left out: S/PDIF (no WM8804 task in V2 builds, see taskMoboCtrl.c), skip/insert, silence detection, rate
 * storage, ADC, volume, prime check, pulses, I2S metadata, FEATURE_UNINVERT_LRCK, full-speed and old Linux quirks.
 *
 * USB PATH, once per receive buffer (EP_OUT_BUFS ring, 2 packets = 250us each)
 * 1. Feedback endpoint as Legacy: FB_rate (16.16 per 250us) >> 1 per 125us packet, nominal while we don't play.
 * 2. Take: the first buffer while nobody owns the output takes it. Without S/PDIF there is nobody to compete with,
 *    so no sound detection. Then clocks and spk_index half a buffer away from the I2S DMA.
 * 3. Gap: every SPK_PACKETS_PER_GAP_CALCULATION buffers, FB_rate steps with Legacy's limits. Beyond the outer
 *    limits 2 steps, but no skip/insert: every sample from the host gets to the DAC.
 * 4. The parse writes straight into spk_buffer, no cache_unified, no second loop. One loop per format:
 *    32 bit: one word load and swap.b per sample. 16 bit: one word load and swap.bh per frame.
 *    24 bit: three halfword loads per frame, one swap.bh per sample.
 * 5. The ring wrap splits the buffer into at most two runs. PX30 toggles and usb_buffer_toggle counts down once
 *    at the split, not after a test of every index.
 * 6. Give: alternate setting 0 ('>'), or the I2S DMA wrapped too often without us writing ('q' ')').
 *    A paused host that keeps streaming zeros keeps the output, and we play its zeros.
 *
 * DEBUG
 * PA22 high while samples are handled. PX30 toggles at each spk_buffer wrap. '?' receive overrun, '~' receive ring
 * resync, '[' take, then the rate digit (print_cpu_char), 'a' / 'm' and ",16" ",24" ",32".
 */

//_____  I N C L U D E S ___________________________________________________

#include <stdio.h>
#include "usart.h"     // Shall be included before FreeRTOS header files, since 'inline' is defined to ''; leading to
                       // link errors
#include "conf_usb.h"


#if USB_DEVICE_FEATURE == ENABLED

#include "board.h"
#include "features.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pdca.h"
#include "gpio.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "uac2_usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "uac2_usb_specific_request.h"
#include "device_audio_task.h"
#include "uac2_device_audio_task.h"
#include "composite_widget.h"
#include "taskAK5394A.h"
#include "Mobo_config.h"

#ifndef HW_GEN_SPRX
	#error "uac2_device_audio_task_V2.c is for SPRX hardware only. Use uac2_device_audio_task_Legacy.c"
#endif


//_____ D E F I N I T I O N S ______________________________________________

#define FB_RATE_DELTA		64			// Feedback step, as Legacy

#define V2_BYTES_16			2			// Bytes per sample of each USB format
#define V2_BYTES_24			3
#define V2_BYTES_32			4


//_____ D E C L A R A T I O N S ____________________________________________

static U8 ep_audio_out_fb;
static U32 ep_out_overruns_seen = 0;		// ep_out_dma_overruns at the last '?'

static S32 spk_index = 0;					// Next spk_buffer word to write, always even (L)
static S32 old_gap = SPK_GAP_NOM;
static S16 time_to_calculate_gap = 0;
static Bool return_to_nominal = FALSE;


//_____ H E L P E R S ______________________________________________________

// Byte order of a word reversed: one little endian USB sample to an AVR32 (big endian) S32
static inline __attribute__((always_inline)) U32 v2_swap_b(U32 x) {
	__asm__ ("swap.b\t%0" : "+r" (x));
	return x;
}

// Bytes swapped within each halfword
static inline __attribute__((always_inline)) U32 v2_swap_bh(U32 x) {
	__asm__ ("swap.bh\t%0" : "+r" (x));
	return x;
}

// spk_index reached the end of spk_buffer
static inline __attribute__((always_inline)) void v2_wrapped(void) {
	gpio_tgl_gpio_pin(AVR32_PIN_PX30);			// Ideally 90 or 270 deg out of phase with the I2S DMA
	usb_buffer_toggle--;						// Increased by the I2S DMA, decreased here
}

// One run of USB frames with no wrap inside. bytes is a constant at every call, so each call compiles to its
// own loop without format branches
static inline __attribute__((always_inline)) void v2_usb_run(const U8 bytes, const U8 **src, S32 **dst, U32 frames) {
	const U8 *s = *src;
	S32 *d = *dst;
	U32 w, u0, u1, u2;

	while (frames != 0) {
		frames--;
		if (bytes == V2_BYTES_32) {				// L0 L1 L2 L3 R0 R1 R2 R3, LSB first. Word aligned
			d[0] = (S32)v2_swap_b(((const U32 *)s)[0]);
			d[1] = (S32)v2_swap_b(((const U32 *)s)[1]);
			s += 8;
		}
		else if (bytes == V2_BYTES_16) {		// L0 L1 R0 R1. Word aligned
			w = v2_swap_bh(*(const U32 *)s);	// L1 L0 R1 R0
			d[0] = (S32)(w & 0xFFFF0000);
			d[1] = (S32)(w << 16);
			s += 4;
		}
		else {									// L0 L1 L2 R0 R1 R2. Halfword aligned
			u0 = ((const U16 *)s)[0];			// L0 L1
			u1 = ((const U16 *)s)[1];			// L2 R0
			u2 = ((const U16 *)s)[2];			// R1 R2
			d[0] = (S32)( ((u1 & 0xFF00) << 16) | (v2_swap_bh(u0) << 8) );		// L2 L1 L0 00
			d[1] = (S32)( (v2_swap_bh(u2) << 16) | ((u1 & 0x00FF) << 8) );		// R2 R1 R0 00
			s += 6;
		}
		d += 2;
	}
	*src = s;
	*dst = d;
}

// One USB buffer into spk_buffer from spk_index, in at most two runs split at the wrap
static inline __attribute__((always_inline)) void v2_usb_put(const U8 bytes, const U8 *src, U32 frames) {
	S32 *dst = (S32 *)&spk_buffer[spk_index];	// Not volatile in the loop, see the barrier after it
	U32 room = (DAC_BUFFER_UNI - spk_index) >> 1;	// Frames before the wrap

	if (frames < room) {
		v2_usb_run(bytes, &src, &dst, frames);
		spk_index += frames << 1;
	}
	else {
		v2_usb_run(bytes, &src, &dst, room);
		v2_wrapped();
		dst = (S32 *)&spk_buffer[0];
		frames -= room;
		v2_usb_run(bytes, &src, &dst, frames);
		spk_index = frames << 1;
	}
	asm volatile ("" ::: "memory");				// The I2S DMA reads spk_buffer, GCC can't see that
}

#ifdef FEATURE_PRIME_CHECK
	// 20261126: Legacy's bit-exact test tools (FEATURE_PRIME_CHECK, debug 'O' 'E' 'Z'), as a separate pass over the
	// frames just written to spk_buffer. The copy loop stays as fast as without them, and the check sees what the
	// I2S DMA will send. Inside PA22, so their cost shows. See Legacy for the full description. In short:
	// PRIME_MODE_CHECK: each channel must step by its prime per frame (tools/make_test_wavs.py). '<' when the first
	//   frame of a buffer breaks, '>' when a later one does. All-zero frames restart the check. Logged for 'E'.
	// PRIME_MODE_PULSE: the frames are overwritten with a fixed pattern of about 0.33us SDATA pulses. '!' when a word
	//   the I2S DMA read since the last buffer doesn't hold the pattern
	// 'O' steps prime_mode 0 1 2 3 in these builds, so the clock mode stays at the boot default 'm'.
	// Keep these primes equal to those in tools/make_test_wavs.py
	#define PRIME_L_16	251
	#define PRIME_R_16	241
	#define PRIME_L_24	65521
	#define PRIME_R_24	65537
	#define PRIME_L_32	16777213
	#define PRIME_R_32	16777259

	static U32 prime_last_L = 0;	// Previous frame, left aligned in 32 bits
	static U32 prime_last_R = 0;
	static Bool prime_synced = FALSE;
	static char prime_break = 0;	// Character to report for this buffer, 0 for none
	static U16 prime_len = 0;		// This buffer's length in bytes
	static U16 prime_prev_len = 0;	// Previous buffer's length in bytes
	static U16 prime_packets = 0;	// Buffers since the previous break, saturating
	static Bool prime_log_pending = FALSE;	// A record waits for its next frame
	static U8 prime_log_slot = 0;	// The record that waits
	static S32 ram_check_pos = -1;	// Next spk_buffer word to check against the pulse pattern, -1 for none

	// Check one frame, as Legacy. step_L and step_R are the primes shifted to the sample's bit position
	static void prime_check_frame(U32 sample_L, U32 sample_R, U32 step_L, U32 step_R, U8 i) {
		volatile U32 *rec;

		if (prime_log_pending) {	// This frame is the next frame of the last logged break
			rec = &prime_log[prime_log_slot * PRIME_LOG_WORDS];
			rec[7] = sample_L;
			rec[8] = sample_R;
			if (prime_log_count == prime_log_slot) {	// Not emptied by 'E' meanwhile: the record is complete
				prime_log_count = prime_log_slot + 1;
			}
			prime_log_pending = FALSE;
		}

		if ( (sample_L == 0) && (sample_R == 0) ) {		// Silence restarts the check
			prime_synced = FALSE;
			return;
		}

		if ( (prime_synced) && (prime_break == 0) &&
			 ( (sample_L - prime_last_L != step_L) || (sample_R - prime_last_R != step_R) ) ) {
			prime_break = (i == 0) ? '<' : '>';

			if ( (!prime_log_frozen) && (prime_log_count < PRIME_LOG_EVENTS) ) {	// Full log waits for 'E'
				prime_log_slot = prime_log_count;
				rec = &prime_log[prime_log_slot * PRIME_LOG_WORDS];
				rec[0] = 0xFFFFFFFF;
				rec[1] = ((U32)prime_break << 24) | ((U32)i << 16) | prime_len;
				rec[2] = ((U32)prime_prev_len << 16) | prime_packets;
				rec[3] = prime_last_L;
				rec[4] = prime_last_R;
				rec[5] = sample_L;
				rec[6] = sample_R;
				prime_log_pending = TRUE;			// Counted when the next frame arrives
			}
			prime_packets = 0;
		}

		prime_last_L = sample_L;
		prime_last_R = sample_R;
		prime_synced = TRUE;
	}

	// The test tools on the frames just written: from spk_buffer[first], frames long, may wrap
	static void v2_prime_buffer(S32 first, U32 frames, U32 len, U8 bytes) {
		const U8 mode = prime_mode;					// Sampled once per buffer
		S32 k;
		U32 i;

		prime_break = 0;
		prime_prev_len = prime_len;
		prime_len = len;
		if (prime_packets < 0xFFFF) {
			prime_packets++;
		}

		if (mode & PRIME_MODE_CHECK) {
			U32 step_L = PRIME_L_32;				// 32-bit samples use all bits
			U32 step_R = PRIME_R_32;
			if (bytes == V2_BYTES_24) {				// 24-bit samples sit in bits 31:8
				step_L = (U32)PRIME_L_24 << 8;
				step_R = (U32)PRIME_R_24 << 8;
			}
			else if (bytes == V2_BYTES_16) {		// 16-bit samples sit in bits 31:16
				step_L = (U32)PRIME_L_16 << 16;
				step_R = (U32)PRIME_R_16 << 16;
			}
			k = first;
			for (i = 0; i < frames; i++) {
				prime_check_frame((U32)spk_buffer[k], (U32)spk_buffer[k + 1], step_L, step_R, (U8)i);
				k += 2;
				if (k >= DAC_BUFFER_UNI) {
					k = 0;
				}
			}
		}
		else {
			prime_synced = FALSE;					// Turning the check on starts a fresh sequence
			prime_log_pending = FALSE;				// and drops a record that waits for its next frame
		}

		if (mode & PRIME_MODE_PULSE) {
			S32 pulse_word;
			if (spk_current_freq.frequency < 60000) {
				pulse_word = 0x55555555;			// 1-bit pulses at 44.1k and 48k
			}
			else if (spk_current_freq.frequency < 120000) {
				pulse_word = 0x33333333;			// 2-bit pulses at 88.2k and 96k
			}
			else if (spk_current_freq.frequency < 240000) {
				pulse_word = 0x0F0F0F0F;			// 4-bit pulses at 176.4k and 192k
			}
			else {
				pulse_word = 0x3FC03FC0;			// 8-bit pulses at 352.8k and 384k, off the byte boundaries
			}

			k = first;
			for (i = 0; i < frames; i++) {
				spk_buffer[k] = pulse_word;
				spk_buffer[k + 1] = pulse_word;
				k += 2;
				if (k >= DAC_BUFFER_UNI) {
					k = 0;
				}
			}

			// RAM check, as Legacy: every word the I2S DMA read since the last buffer must hold the pattern. The writer
			// is half a buffer ahead, so those words are still what the DMA sent. Old data gives '!' for one buffer lap
			// after the mode is turned on
			S32 dma_pos = DAC_BUFFER_UNI - spk_pdca_channel->tcr;	// Next word the DMA reads
			Bool ram_bad = FALSE;
			if (dma_pos >= DAC_BUFFER_UNI) {
				dma_pos -= DAC_BUFFER_UNI;
			}
			if (ram_check_pos >= 0) {
				while (ram_check_pos != dma_pos) {
					if (spk_buffer[ram_check_pos] != pulse_word) {
						ram_bad = TRUE;
					}
					ram_check_pos++;
					if (ram_check_pos >= DAC_BUFFER_UNI) {
						ram_check_pos = 0;
					}
				}
			}
			ram_check_pos = dma_pos;
			if (ram_bad) {
				print_dbg_char('!');
			}
		}
		else {
			ram_check_pos = -1;
		}

		// Debug 'Z': 0 into L and R of the last frame written
		if (spk_zero_frame) {
			k = spk_index - 2;
			if (k < 0) {
				k += DAC_BUFFER_UNI;
			}
			spk_buffer[k] = 0;
			spk_buffer[k + 1] = 0;
			spk_zero_frame = FALSE;
		}
	}
#endif


// spk_index half a buffer away from the I2S DMA's read head
static void v2_init_spk_index(void) {
	spk_index = DAC_BUFFER_UNI - (spk_pdca_channel->tcr) + DAC_BUFFER_UNI / 2;
	spk_index &= ~1L;							// Start with L
	if (spk_index >= DAC_BUFFER_UNI) {
		spk_index -= DAC_BUFFER_UNI;
	}
	else if (spk_index < 0) {
		spk_index += DAC_BUFFER_UNI;
	}
	old_gap = SPK_GAP_NOM;
	return_to_nominal = FALSE;
	time_to_calculate_gap = 0;
}

// Gap between spk_index and the I2S DMA, every SPK_PACKETS_PER_GAP_CALCULATION buffers, steers FB_rate.
// Legacy's limits and steps, without the skip/insert beyond the outer limits
static void v2_gap(void) {
	S32 gap;

	if (time_to_calculate_gap > 0) {
		time_to_calculate_gap--;
		return;
	}
	time_to_calculate_gap = SPK_PACKETS_PER_GAP_CALCULATION - 1;

	gap = DAC_BUFFER_UNI - spk_index - (spk_pdca_channel->tcr);
	if (gap < 0) {
		gap += DAC_BUFFER_UNI;
	}

	if (gap < old_gap) {
		if (gap < SPK_GAP_LSKIP) {				// Outer lower bound: 2 steps
			FB_rate -= 2 * FB_RATE_DELTA;
			old_gap = gap;
			print_cpu_char(CPU_CHAR_DECDEC_FREQ);
			return_to_nominal = TRUE;
		}
		else if (gap < SPK_GAP_L1) {			// Inner lower bound: 1 step
			FB_rate -= FB_RATE_DELTA;
			old_gap = gap;
			print_cpu_char(CPU_CHAR_DEC_FREQ);
			return_to_nominal = TRUE;
		}
		else if ( (gap < SPK_GAP_NOM) && (return_to_nominal) ) {	// Back towards nominal without overshoot
			FB_rate -= FB_RATE_DELTA;
			old_gap = gap;
			print_cpu_char(CPU_CHAR_NOMDEC_FREQ);
			return_to_nominal = FALSE;
		}
	}
	else if (gap > old_gap) {
		if (gap > SPK_GAP_USKIP) {				// Outer upper bound: 2 steps
			FB_rate += 2 * FB_RATE_DELTA;
			old_gap = gap;
			print_cpu_char(CPU_CHAR_INCINC_FREQ);
			return_to_nominal = TRUE;
		}
		else if (gap > SPK_GAP_U1) {			// Inner upper bound: 1 step
			FB_rate += FB_RATE_DELTA;
			old_gap = gap;
			print_cpu_char(CPU_CHAR_INC_FREQ);
			return_to_nominal = TRUE;
		}
		else if ( (gap > SPK_GAP_NOM) && (return_to_nominal) ) {
			FB_rate += FB_RATE_DELTA;
			old_gap = gap;
			print_cpu_char(CPU_CHAR_NOMINC_FREQ);
			return_to_nominal = FALSE;
		}
	}
}


//_____ U S B ______________________________________________________________

static void v2_usb_feedback(void) {
	U32 fb_value;

	if (!Is_usb_in_ready(ep_audio_out_fb)) {
		return;
	}
	Usb_ack_in_ready(ep_audio_out_fb);
	Usb_reset_endpoint_fifo_access(ep_audio_out_fb);

	// The host only gets our feedback while we play its data, else the nominal rate
	fb_value = (input_select == MOBO_SRC_UAC2) ? FB_rate : FB_rate_nominal;
	fb_value = fb_value >> 1;					// 16.16 per 250us to 16.16 per 125us packet
	Usb_write_endpoint_data(ep_audio_out_fb, 8, fb_value);
	Usb_write_endpoint_data(ep_audio_out_fb, 8, fb_value >> 8);
	Usb_write_endpoint_data(ep_audio_out_fb, 8, fb_value >> 16);
	Usb_write_endpoint_data(ep_audio_out_fb, 8, fb_value >> 24);
	Usb_send_in(ep_audio_out_fb);
}

// The take: playback starts at the default MCU speed, mobo_mcu_clock() locks to MCLK in MCU_MODE_MCLK
static void v2_usb_take(U8 bytes) {
	if ( (INPUT_HOLD_ACTIVE()) && (input_select_preferred != INPUT_USB_PLUG_SRC()) ) {
		return;									// During an input select hold, only the held USB plug may take
	}
	if (xSemaphoreTake(input_select_semphr, 10) != pdTRUE) {
		print_dbg_char('*');
		print_dbg_char('a');
		return;
	}
	input_select = MOBO_SRC_UAC2;
	print_dbg_char('[');
	print_cpu_char( (usb_ch == USB_CH_B) ? CPU_CHAR_UAC2_B : CPU_CHAR_UAC2_C );

	FB_rate = FB_rate_initial;
	usb_buffer_toggle = 0;
	dac_must_clear = DAC_READY;					// Prepare to send actual data to DAC interface

	mobo_xo_select(spk_current_freq.frequency, input_select);
	if (mobo_mcu_clock(mobo_mcu_clock_for(spk_current_freq.frequency)) != MCU_FREQ_DEFAULT) {
		print_dbg_char(MCU_MODE_MCLK);
	}
	else {
		print_dbg_char(MCU_MODE_ASYNC);
	}
	mobo_clock_division(spk_current_freq.frequency);
	mobo_led_select(spk_current_freq.frequency, input_select);
	print_dbg_char(',');
	print_dbg_char('0' + (bytes * 8) / 10);		// ",16" ",24" ",32"
	print_dbg_char('0' + (bytes * 8) % 10);

	v2_init_spk_index();						// After the clocks, which take milliseconds
	#ifdef FEATURE_PRIME_CHECK
		prime_synced = FALSE;					// A new stream may start anywhere in a test file
	#endif
}

// Give input_select back. The MCU goes back to the default speed first
static void v2_usb_give(char c, char err) {
	if (input_select != MOBO_SRC_UAC2) {
		return;									// Not ours: the MCU is at the default speed already
	}
	mobo_mcu_clock(MCU_FREQ_DEFAULT);
	mobo_clear_dac_channel();					// Leave the DAC buffer empty as we check out
	if (xSemaphoreGive(input_select_semphr) == pdTRUE) {
		print_dbg_char(c);
		print_dbg_char('\n');
		print_cpu_char(CPU_CHAR_IDLE);
		INPUT_HOLD_START(INPUT_USB_PLUG_SRC(), INPUT_HOLD_PERIODS_USB);	// Only this USB plug may retake for a while
		input_select = MOBO_SRC_NONE;
	}
	else {
		print_dbg_char('*');
		print_dbg_char(err);
	}
}

// One receive buffer from the EP_OUT_BUFS ring
static void v2_usb_buffer(U8 buf) {
	const U8 *src = (const U8 *)ep_out_ram + buf * EP_OUT_BUF_BYTES;
	U32 len = ep_out_buf_len[buf];				// Bytes
	U32 frames = 0;
	U8 bytes = 0;

	gpio_set_gpio_pin(AVR32_PIN_PA22);			// Start of sample handling
	asm volatile ("" ::: "memory");				// ep_out_ram was written by the DMA. Reload after the flag

	if (usb_alternate_setting_out == ALT1_AS_INTERFACE_INDEX) {
		bytes = V2_BYTES_24;
		frames = len / 6;
	}
	else if (usb_alternate_setting_out == ALT2_AS_INTERFACE_INDEX) {
		bytes = V2_BYTES_16;
		frames = len >> 2;
	}
	else if (usb_alternate_setting_out == ALT3_AS_INTERFACE_INDEX) {
		bytes = V2_BYTES_32;
		frames = len >> 3;
	}
	spk_usb_heart_beat++;						// EP_AUDIO_OUT receives data from the host

	if ( (input_select == MOBO_SRC_NONE) && (frames != 0) ) {
		v2_usb_take(bytes);						// Milliseconds: expect '?' and '~' after it
	}

	if ( (input_select == MOBO_SRC_UAC2) && (frames != 0) ) {
		#ifdef FEATURE_PRIME_CHECK
			S32 first = spk_index;				// Where this buffer's frames start
		#endif
		v2_gap();								// Before the parse: needs only spk_index and the DMA's tcr
		switch (bytes) {
			case V2_BYTES_32: v2_usb_put(V2_BYTES_32, src, frames); break;
			case V2_BYTES_24: v2_usb_put(V2_BYTES_24, src, frames); break;
			default:          v2_usb_put(V2_BYTES_16, src, frames); break;
		}
		#ifdef FEATURE_PRIME_CHECK
			v2_prime_buffer(first, frames, len, bytes);
		#endif
	}

	// Done with this buffer, hand it back to the DMA
	ep_out_buf_ready[buf] = FALSE;
	ep_out_parse_buf = EP_OUT_BUF_NEXT(buf);
	gpio_clr_gpio_pin(AVR32_PIN_PA22);			// End of sample handling

	if (ep_out_dma_overruns != ep_out_overruns_seen) {	// The DMA wrote into a buffer before its parse
		ep_out_overruns_seen = ep_out_dma_overruns;
		print_dbg_char('?');
	}

	#ifdef FEATURE_PRIME_CHECK
		if (prime_break != 0) {					// After PA22, so a waiting UART can't delay the parse
			print_dbg_char(prime_break);
			prime_break = 0;
		}
	#endif
}


//_____ T A S K ____________________________________________________________

void uac2_device_audio_task_init(U8 ep_in, U8 ep_out, U8 ep_out_fb)
{
	(void)ep_in;								// No ADC
	(void)ep_out;								// Received by DMA, see ep_out_dma_isr()
	ep_audio_out_fb = ep_out_fb;

	xTaskCreate(uac2_device_audio_task,
				configTSK_USB_DAUDIO_NAME,
				configTSK_USB_DAUDIO_STACK_SIZE,
				NULL,
				configTSK_USB_DAUDIO_PRIORITY,
				NULL);
}


void uac2_device_audio_task(void *pvParameters)
{
	portTickType xLastWakeTime = xTaskGetTickCount();

	while (TRUE) {
		vTaskDelayUntil(&xLastWakeTime, UAC2_configTSK_USB_DAUDIO_PERIOD);

		if (dac_must_clear == DAC_MUST_CLEAR) {
			mobo_clear_dac_channel();
			dac_must_clear = DAC_CLEARED;
		}

		if ( (usb_alternate_setting_out >= 1) && (usb_ch_swap == USB_CH_NOSWAP) ) {
			v2_usb_feedback();

			// Receive ring resync: ready buffers start at ep_out_parse_buf. If that one isn't ready but another is,
			// the parse lost track (DMA restart, or a take that held us for milliseconds). Follow the DMA
			if (!ep_out_buf_ready[ep_out_parse_buf]) {
				U8 b = ep_out_parse_buf;
				U8 k;
				for (k = 1; k < EP_OUT_BUFS; k++) {
					b = EP_OUT_BUF_NEXT(b);
					if (ep_out_buf_ready[b]) {
						ep_out_parse_buf = b;
						print_dbg_char('~');
						break;
					}
				}
			}

			if (ep_out_buf_ready[ep_out_parse_buf]) {
				v2_usb_buffer(ep_out_parse_buf);
			}
		}
		else {													// No USB stream, or a USB channel swap
			if (usb_ch_swap == USB_CH_SWAPDET) {
				usb_ch_swap = USB_CH_SWAPACK;					// Acknowledge, takes this task into startup
			}
			v2_usb_give('>', 'c');								// Returns at once unless we play
		}

		// The I2S DMA wrapped too often without us writing: USB stopped feeding
		if (usb_buffer_toggle == USB_BUFFER_TOGGLE_LIM) {
			usb_buffer_toggle = USB_BUFFER_TOGGLE_PARK;			// Park this mechanism
			print_dbg_char('q');
			v2_usb_give(')', 'd');
		}
	}
}

#endif  // USB_DEVICE_FEATURE == ENABLED
