/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */

/* Written by Borge Strand-Bergesen 20150701
 * Interaction with WM8805
 *
 * Copyright (C) Borge Strand-Bergesen, borge@henryaudio.com
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


/*
Using the WM8805 as an SPDIF receiver in software control mode

Here is an engineer's viewpoint and experience on programming the WM8805. I've decided
to share how I got this chip to work well on all relevant audio sample rates (including
176.4 and 192ksps). Hopefully, this will be a good complement to the datasheet. You may
ask why I put this together. Basically, it was harder than the toughest suduko I've
ever solved, and it felt much more meaningful. I may put the technology into a product
one day in the future, but until then it's a very nice addition to my design toolbox.

The code can be found at:
https://github.com/borgestrand/sdr-widget/blob/audio-widget-experimental/src/wm8805.c

The WM8805 from Wolfson / Cirrus is a multi-port SPDIF receiver and SPDIF transmitter
for consumer mode signaling. (I.e. no AES/EBU.) Its main feature is a digital fractional
PLL for clock recovery. Most other SPDIF receivers use an analog PLL structure which
depends on external passive components. Proper listening tests will come next.

This work on programming the WM8805 was is add-on to the Audio Widget project. Just like
the Audio Widget project, the commercially available Henry Audio USB DAC 128 mkII and the
SDR-Widget project, source code is open source.

The WM8805 and the WM8804 are difficult to use. But the good news is that the code base
here makes the WM8805 work very well with two different SPDIF sources and all sample
rates. I have described some shortcomings of the chip, but to a great extent they can be
overcome. The code handles automatic detection and re-configuration of the notorious
176.4ksps sample rate along with 44.1, 48, 88.2, 96 and 192ksps. Another piece of good
news is that the I2C / two-wire control worked right out of the box with the routines in
the AVR32 code base. That's a first for me. I'm a bit too used to bit-banging poorly
implemented I2C....

The hardware has been tested with SPDIF and TOSLINK inputs up to 192ksps. The hardware
is essentially a modified Henry Audio USB DAC 128 mkII design where a few IO lines
have been reassigned. For an experienced electrical engineer, detailed knowledge of the
hardware platform is probably not needed in order to port this design.

The devil is in the details. The sample rate detector of the WM8805 uses the same status
information to encode 44.1 and 48ksps, the same for 88.2 and 96, and the same for 176.4
and 192. In itself this isn't that bad, if it wasn't for the fact that 192ksps needs a
different initial PLL setting from the other five. So there's basically no way to
determine from the chip itself whether that setting is needed. The sample rate detector
inside the WM8805 can't be trusted, and neither can its sample rate change interrupt.
(Yes, I did try with an averaged one-size-fits-all PLL setting. That ended up not
working for any sample rate...)

Luckily, the WM8805 sends out an I2S word clock of the correct sample rate, even though
the PLL isn't correctly set up. This means the external sample rate detector can be used
to determine which PLL setting to apply. The WM8805 interrupt is polled in order to
determine whether it has lost lock.

The pivotal part of the code is the external sample rate detector. It was programmed in
AVR32 assembly code. (Yes, I cheated and looked at the .asm which some sample .c compiled
into. With that as a starting point and a lot of time having coded other assembly languages
it was easy hacking even without knowledge of this particular machine language.) The delays
were verified with a square wave generator and later with actual I2S. Change the MCU
frequency and all the delays must be redone.

The FreeRTOS scheduler of the Audio Widget project is halted for a bit more than one
audio word clock period, during which time a counter is decremented. If you wish to
port this effort to a different platform, the first thing to do is determine if have
the resources available to implement a good sample rate detector. Then you must
understand the function wm8805_srd(). The .c -> .asm recipe is in its comments.

Because the starting point for the design is an USB DAC, there is code in place to make
the WM8805 and the MCU based USB audio receiver share one I2S interface to the DAC. The
RTOS software uses a semaphore to determine which task (WM8805 monitor or USB audio) may
control the I2S and MCLK multiplexers. If your goal is to only use the WM8805 part of
the design, the semaphore code can be easily designed out.

Another shortcoming of the WM8805 is the lack of a user controlled mute function. I'd like
it to generate some functional and muted I2S when it's trying to obtain lock or doing
other things without valid digital audio arriving. There is no soft mute or anti-pop
functionality which one can pretty much expect in modern digital audio. Instead the code
switches in the USB receiver's muted I2S output whenever a WM8805 mute is needed. The USB
receiver's output buffer is often cleared in the USB code. That means there are few
explicit calls to wm8805_mute(). And those that are there can probably be removed.

The current implementation of wm8805_mute() is crude. At least it should wait until a
sample boundary before setting the I2S data line to 0. That is because a small negative
value truncated with zeros will become a large negative value to the DAC. With no control
of the exact time when the USB's I2S output replaces that of the WM8805, this condition is
hard to prevent, and some pops may be expected. A modest hardware fix would be to have an
AND gate zero the I2S data line, and have its control signal be latched by a negative
transition of the word clock. A fuller software fix would be to route the WM8805's I2S
output into the MCU's ADC interface so that gentle muting and I2S multiplexing can be done
in software. My prototype hardware is prepared for this option but I haven't yet begun
building it.

The WM8805 in this design supports two SPDIF inputs (one over a TOSLINK plug). When USB
audio is playing the WM8805 is shut down. But when that is not the case, the WM8805 scans
its inputs until it finds one with a lock. With lock it sends that input to the DAC's I2S
input. The WM8805 spends 200ms or so trying to lock to one input before it gives up and
tries the next. This can easily be extended to more than two SPDIF inputs into the WM8805.
This automatic input selector is working well. The only flipside is that it doesn't respond
immediately to digital audio appearing on an input. If that is desired, the number of inputs
must be reduced, or the user must press a button for channel selection.

While the scan mode is quick, the code takes more time to give up on a WM8805 or USB when
there has been actual audio coming from that interface. Only when the interface looses
lock or becomes muted for more than 2s does the code start looking at other inputs for
valid audio.

This mechanism also extends to the USB interface. With input_select == MOBO_SRC_UAC2 or
MOBO_SRC_UAC1 the WM8805 is powered down and the USB receiver owns the DAC's I2S interface.
When the USB audio is muted or halted, input_select is set to MOBO_SRC_NONE and the WM8805
is powered up to start scanning its inputs. When it finds a valid one, input_select is set
to MOBO_SRC_SPDIF or MOBO_SRC_TOS2, and the I2S interface is switched over to the WM8805.
And when the WM8805 is muted or looses link, the I2S is once again handed over to the USB
system. At first, with input_select == MOBO_SRC_NONE, this is done to mute the DAC.

If this project is of interest to you, please let me know! I hope to see you at either:
 Audio Widget mailing list: audio-widget@googlegroups.com
 https://www.facebook.com/henryaudio
 borge@henryaudio.com

*/



// #if defined(HW_GEN_DIN10)									// Functions here only make sense for WM8805
#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)

#include "wm8805.h"
// #include "Mobo_config.h"
#include "features.h"
#include "device_audio_task.h"
#include "usb_specific_request.h"
#include "Mobo_config.h"
#include "I2C.h"
#include "pdca.h" // To disable DMA at sleep
#include "taskAK5394A.h" // To signal uacX_device_audio_task to enable DMA at init

// Global status variable
volatile wm8805_status_t wm8805_status = {0, 1, 0, 0, FREQ_TIMEOUT, WM8805_PLL_NONE, 1};

//!
//! @brief Polling routine for WM8805 hardware
//!
void wm8805_poll(void) {

	// Arbitrary startup delay ATD
	#define pausecounter_initial 20000
	#define startup_empty_runs 40 // Was 40 // Will this help for cold boot?

	/* Probably not the cause of initial startup hassle
	 * 400 - 32s
	 * Brief power down
	 * 400 - 18s
	 * 400 - 14s
	 *
	 */



    uint8_t wm8805_int = 0;										// WM8805 interrupt status
    static uint8_t input_select_wm8805_next = MOBO_SRC_TOS2;	// Try TOSLINK first
	static int16_t pausecounter = pausecounter_initial;						// Initiated to a value much larger than what is seen in practical use
	static int16_t unlockcounter = 0;
	static int16_t lockcounter = 0;
	int16_t pausecounter_temp;
	int16_t unlockcounter_temp;


	// Run the first 40 iterations without starting up. (40*120/10 = 480ms)
	if (pausecounter >= pausecounter_initial - startup_empty_runs) {
		pausecounter--;
		if (pausecounter == pausecounter_initial - startup_empty_runs)
			pausecounter = 0;
		return;
	}


	/* NEXT:
	 * + Decide on which interrupts to enable
	 * + Do some clever bits with silencing
	 * + Prevent USB engine from going bonkers when playing on the WM (buffer zeros and send nominal sample rate...)
	 * + Update USB silence detector timeouts
	 * + Test UAC2 code and port to UAC1
	 * - Tasks to be eliminated, particularly uacX_taskAK5394A.c?
	 * + Test code base on legacy hardware
	 * - Check if WM is really 24 bits
	 * + Test SPDIF
	 * + Structure code away from mobo_config.c/h, device_mouse_hid_task.c etc.
	 * + Is spk_mute ever used in uac2_d_a_t? Yes, for volume control!
	 * + Think about some automatic silence detecting software!
	 * - Long-term testing
	 * + Categorize sample rate detector with proper signal generator, +-2%, reduce timeout constant everywhere
	 * + Determine correct GPIO pin for SRD, recompile for asm constants
	 * + Test USB music playback with SRD running continuously
	 * + Get hardware capable of generating all SPDIF sample rates
	 * + Make state machine for WM8805 sample rate detection
	 * + Make state machine for source selection
	 * + Figure out ADC interface
	 * + Make silence detector (use 1024 silent block detector in WM?)
	 */

/*
	if (gpio_get_pin_value(WM8805_CSB_PIN) == 1)	{	// Not locked! NB: We're considering an init pull-down here...
		if ( (input_select == MOBO_SRC_SPDIF) || (input_select == MOBO_SRC_TOS2) || (input_select == MOBO_SRC_TOS1) ) {
			wm8805_status.reliable = 0;					// Something went wrong, we're not reliable
			wm8805_mute();								// Semi-immediate software-mute? Or almost-immediate hardware mute?
		}
	}
*/


/*
	// Check frequency status. Halt operation if frequency has changed. Don't change source
	freq_temp = wm8805_srd();
	if ( (wm8805_status.frequency != freq_temp) && (freq_temp != FREQ_TIMEOUT) ) {	// Do we have time for this?
		print_dbg_char('\\');

		wm8805_status.frequency = freq_temp;

		wm8805_status.reliable = 0;
		wm8805_mute();
		wm8805_status.muted = 1;
		lockcounter = 0;
		unlockcounter = 0;
	}
*/

	// USB is giving away control,
	if (input_select == MOBO_SRC_NONE) {
		if (wm8805_status.powered == 0) {
			wm8805_init();								// WM8805 was probably put to sleep before this. Hence re-init
			wm8805_status.pllmode = WM8805_PLL_NONE;			// Force PLL re-init
//			print_dbg_char('0');
			wm8805_status.powered = 1;
			wm8805_status.muted = 1;					// I2S is still controlled by USB which should have zeroed it.

			pausecounter = 0;
			unlockcounter = 0;
			lockcounter = 0;

			wm8805_status.reliable = 0;					// Because of input change
			wm8805_input(input_select_wm8805_next);		// Try next input source
//			print_dbg_char('a');

			wm8805_pll();
//			print_dbg_char('b');
		}
	}
	// USB has assumed control, power down WM8805 if it was on
	else if ( (input_select == MOBO_SRC_UAC1) || (input_select == MOBO_SRC_UAC2) ) {
		if (wm8805_status.powered == 1) {
			wm8805_status.powered = 0;
			wm8805_status.reliable = 0;					// Because of power down
			wm8805_sleep();
		}
	}


	// Current WM8805 input is silent or unavailable. When WM is selected, wait for a long time (paused).
	// When WM is not selected, scan WM inputs for a short time (unlinked)
	// Playing input: Tolerate WM8805_PAUSE_LIM of silence before searching for other input.
	// Scanning inputs: Tolerate WM8805_SILENCE_LIM before searching on
	if ( (input_select == MOBO_SRC_SPDIF) || (input_select == MOBO_SRC_TOS2) || (input_select == MOBO_SRC_TOS1) ) {
		unlockcounter_temp = WM8805_HICKUP_LIM;
		pausecounter_temp = WM8805_PAUSE_LIM;
	}
	else {
		unlockcounter_temp = WM8805_UNLOCK_LIM;
		pausecounter_temp = WM8805_SILENCE_LIM;
	}

	if ( (wm8805_status.powered == 1) && ( (unlockcounter >= unlockcounter_temp) || (pausecounter >= pausecounter_temp) ) ) {
		if ( (input_select == MOBO_SRC_SPDIF) || (input_select == MOBO_SRC_TOS2) || (input_select == MOBO_SRC_TOS1) ) {

			wm8805_mute();
			wm8805_status.muted = 1;

#ifdef USB_STATE_MACHINE_DEBUG
			print_dbg_char('G');						// Debug semaphore, capital letters for WM8805 task
			if (xSemaphoreGive(input_select_semphr) == pdTRUE) {

				// Highly experimental
//				gpio_clr_gpio_pin(USB_DATA_ENABLE_PIN_INV);		// Enable USB MUX

				input_select = MOBO_SRC_NONE;				// Indicate USB may  over control, but don't power down!
				print_dbg_char(60); // '<'
			}
			else
				print_dbg_char(62); // '>'
#else
			if (xSemaphoreGive(input_select_semphr) == pdTRUE) {

				// Highly experimental
//				gpio_clr_gpio_pin(USB_DATA_ENABLE_PIN_INV);		// Enable USB MUX

				input_select = MOBO_SRC_NONE;				// Indicate USB may take over control, but don't power down!
			}
#endif
		}

		// Try other WM8805 channel
		if (input_select == MOBO_SRC_NONE) {

			#ifdef HW_GEN_DIN10									// Only SPDIF and TOS2 available
				if (input_select_wm8805_next == MOBO_SRC_TOS2)	// Prepare to probe other WM channel next time we're here
					input_select_wm8805_next = MOBO_SRC_SPDIF;
				else
					input_select_wm8805_next = MOBO_SRC_TOS2;
			#endif
			#ifdef HW_GEN_DIN20									// SPDIF, TOS2 and TOS1 available
				if (input_select_wm8805_next == MOBO_SRC_TOS2)	// Prepare to probe other WM channel next time we're here
					input_select_wm8805_next = MOBO_SRC_TOS1;
				else if (input_select_wm8805_next == MOBO_SRC_TOS1)	// Prepare to probe other WM channel next time we're here
					input_select_wm8805_next = MOBO_SRC_SPDIF;
				else
					input_select_wm8805_next = MOBO_SRC_TOS2;
			#endif

			// FIX: disable and re-enable ADC DMA around here?
			wm8805_status.reliable = 0;						// Because of input change
			wm8805_input(input_select_wm8805_next);			// Try next input source
//			print_dbg_char('c');

			wm8805_pll();
//			print_dbg_char('d');

			lockcounter = 0;
			unlockcounter = 0;
			pausecounter = 0;
		}
	}


	// Check if WM8805 is able to lock and hence play music, only use when WM8805 is powered
	if ( (wm8805_status.muted == 1) && (wm8805_status.reliable == 1) && (pausecounter == 0)) {

		if (input_select == MOBO_SRC_NONE) {		// Semaphore is untaken, try to take it
#ifdef USB_STATE_MACHINE_DEBUG
			print_dbg_char('T');					// Debug semaphore, capital letters for WM8805 task
			if (xSemaphoreTake(input_select_semphr, 0) == pdTRUE) {	// Re-take of taken semaphore returns false
				print_dbg_char('[');
				input_select = input_select_wm8805_next;	// Owning semaphore we may write to input_select

				// Highly experimental
//				gpio_set_gpio_pin(USB_DATA_ENABLE_PIN_INV);		// Disable USB MUX

			}
			else
				print_dbg_char(']');
#else // not debug
			if (xSemaphoreTake(input_select_semphr, 0) == pdTRUE) { // Re-take of taken semaphore returns false

				// Highly experimental
//				gpio_set_gpio_pin(USB_DATA_ENABLE_PIN_INV);		// Disable USB MUX

				input_select = input_select_wm8805_next;	// Owning semaphore we may write to input_select
			}
#endif
		}

		// Do we own semaphore? If so, change I2S setting
		if ( (input_select == MOBO_SRC_SPDIF) || (input_select == MOBO_SRC_TOS2) || (input_select == MOBO_SRC_TOS1) ) {
			wm8805_clkdiv();						// Configure MCLK division
			wm8805_unmute();						// Reconfigure DAC-side I2S and LEDs
			wm8805_status.muted = 0;
		}
	}


	// Polling interrupt monitor, only use when WM8805 is on
	if ( (gpio_get_pin_value(WM8805_INT_N_PIN) == 0) && (wm8805_status.powered == 1) ) {
		wm8805_status.reliable = 0;					// RX is not stable
		if (wm8805_status.muted == 0) {				// Mute audio
			wm8805_mute();
			wm8805_status.muted = 1;
		}

		// 2*50 if clean, up to 20*50 if not clean. PLL change typically uses two interrupts
		int i = 20;
		while (i > 0) {
			vTaskDelay(30);

			if (gpio_get_pin_value(WM8805_CSB_PIN) == 1)
				i--;
			else
				i-=10;
		}

		wm8805_int = wm8805_read_byte(0x0B);		// Record interrupt status and clear pin

//		print_dbg_char('!');
//		print_dbg_char_hex(wm8805_int);

		wm8805_pll();
//		print_dbg_char('e');

	}	// Done handling interrupt


	// Monitor silent or disconnected WM8805 input
	if (wm8805_status.powered == 1) {
		if (gpio_get_pin_value(WM8805_CSB_PIN) == 1) {	// Not locked!
			wm8805_status.reliable = 0;					// Duplication of code from the top of this section, bad style!
			lockcounter = 0;
			if (unlockcounter < WM8805_HICKUP_LIM) {
				unlockcounter++;
			}
		}
		else {											// Lock indication
			unlockcounter = 0;
			if (lockcounter < WM8805_LOCK_LIM) {
				lockcounter++;
			}
			else {
				if (wm8805_status.reliable == 0) {		// Only once at this spot
/*
					#define sdr_poll_repetitions 10
					int i = sdr_poll_repetitions;		// N, number of successful polls required
					wm8805_status.frequency = wm8805_srd();

					while (i > 0) {						// Must verify N times
						vTaskDelay(10);					// 1ms poll
						if (wm8805_status.frequency == wm8805_srd())
							i--;
						else {							// New baseline to be verified
							wm8805_status.frequency = wm8805_srd();
							i = sdr_poll_repetitions;
						}

					}
*/
					wm8805_status.frequency = wm8805_srd();

					if (wm8805_status.frequency != FREQ_TIMEOUT) {
//						print_dbg_char('r');
						wm8805_status.reliable = 1;
					}
				}
			}
		}

// 		SW zero detector depends on uacX_device_audio_task to update .silent
//		if ( (wm8805_status.silent == 1) || (gpio_get_pin_value(WM8805_ZERO_PIN) == 1) ) {
		if ( (wm8805_status.silent == 1)  ) {
			if (pausecounter < WM8805_PAUSE_LIM)
				pausecounter++;
		}
		else {
			pausecounter = 0;
		}
	}

} // wm8805_poll


// Using the WM8805 requires intimate knowledge of the chip and its datasheet. For this
// reason we use a lot of raw hex rather than naming of its internal registers.

// Hardware reset over GPIO pin. Consider the Power Up Configuration section of the datasheet!
void wm8805_reset(uint8_t reset_type) {
	if (reset_type == WM8805_RESET_START) {
		// 20170423 new
		gpio_set_gpio_pin(AVR32_PIN_PA15);			// SDA must be 1 at reset for SW. NB This will conflict with I2C!

		gpio_clr_gpio_pin(WM8805_RESET_PIN);		// Clear reset pin WM8805 active low reset
		gpio_clr_gpio_pin(WM8805_CSB_PIN);			// CSB/GPO2 pin sets 2W address. Make sure CSB outputs 0.
	}
	else {
		gpio_set_gpio_pin(WM8805_RESET_PIN);		// Set reset pin WM8805 active low reset
		gpio_enable_gpio_pin(WM8805_CSB_PIN);		// CSB/GPO2 should now be an MCU input...

		// 20170423 new
		gpio_enable_gpio_pin(AVR32_PIN_PA15);		// SDA should now be an MCU input...
	}
}

// Start up the WM8805
void wm8805_init(void) {

// Arbitrary startup delay ATD
//	static uint8_t initial = 1;

	wm8805_write_byte(0x08, 0x70);	// 7:0 CLK2, 6:1 auto error handling disable, 5:1 zeros@error, 4:1 CLKOUT enable, 3:0 CLK1 out, 2-0:000 RX0

	wm8805_write_byte(0x1C, 0xCE);	// 7:1 I2S alive, 6:1 master, 5:0 normal pol, 4:0 normal, 3-2:11 or 10 24 bit, 1-0:10 I2S ? CE or CA ?

	wm8805_write_byte(0x1D, 0xC0);	// 7 SPD_192K_EN = 1, Change 6:1, disable data truncation, run on 24 bit I2S

	wm8805_write_byte(0x17, 0x00);	// 7:4 GPO1=INT_N (=SPIO_00, PX54), 3:0 GPO0=INT_N, that pin has 10kpull-down

	wm8805_write_byte(0x18, 0xF7);	// 7:4 GPO3='0', 3:0 GPO2=UNLOCK, WM8805_CSB_PIN=PX37 on HW_GEN_DIN20. OK with initial software set to 1?

	wm8805_write_byte(0x1A, 0xCF);	// 7:4 GPO7=ZEROFLAG (=SPIO_04, PX15), 3:0 GPO6=0, that pin is grounded SPDIF in via write to 0x1D:5
//	wm8805_write_byte(0x1A, 0xC0);	// 7:4 GPO7=ZEROFLAG (=SPIO_04, PX15), 3:0 GPO6=INT_N, that pin is grounded SPDIF in via write to 0x1D:5
//	wm8805_write_byte(0x1A, 0x70);	// 7:4 GPO7=UNLOCK (=SPIO_04, PX15), 3:0 GPO6=INT_N, that pin is grounded SPDIF in via write to 0x1D:5
//	wm8805_write_byte(0x1A, 0x40);	// 7:4 GPO7=TRANS_ERR (=SPIO_04, PX15), 3:0 GPO6=INT_N, that pin is grounded SPDIF in via write to 0x1D:5

	wm8805_write_byte(0x0A, 0b11100100);	// REC_FREQ:mask (broken in wm!), DEEMPH:ignored, CPY:ignored, NON_AUDIO:active
											// TRANS_ERR:active, CSUD:ignored, INVALID:active, UNLOCK:active

	wm8805_read_byte(0x0B);			// Clear interrupts

	wm8805_write_byte(0x1E, 0x1B);	// Power down 7-6:0, 5:0 OUT, 4:1 _IF, 3:1 _OSC, 2:0 TX, 1:1 _RX, 0:1 _PLL,

// Arbitrary startup delay ATD
//	if (initial == 1) {
//		vTaskDelay(1000);
//		initial = 0;
//	}

	// Enable CPU's processing of produced data
	// This is needed for the silence detector
	AK5394A_pdca_rx_enable(FREQ_INVALID);	// Start up without caring about I2S frequency or synchronization

//	pdca_enable(PDCA_CHANNEL_SSC_RX);			// Enable I2S reception at MCU's ADC port
//  pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
}

// Turn off wm8805, why can't we just run init again?
void wm8805_sleep(void) {
	pdca_disable(PDCA_CHANNEL_SSC_RX);				// Disable I2S reception at MCU's ADC port
	pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);

	wm8805_write_byte(0x1E, 0x1F);	// Power down 7-6:0, 5:0 OUT, 4:1 _IF, 3:1 _OSC, 2:1 _TX, 1:1 _RX, 0:1 _PLL,
}

// Select input channel of the WM8805
void wm8805_input(uint8_t input_sel) {

//	print_dbg_char(0x30 + input_sel);

	wm8805_write_byte(0x1E, 0x06);		// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:1 _RX, 0:0 PLL,

	if (input_sel == MOBO_SRC_TOS2) {
		wm8805_write_byte(0x08, 0x34);	// 7:0 CLK2, 6:0 auto error handling enable, 5:1 zeros@error, 4:1 CLKOUT enable, 3:0 CLK1 out, 2-0:4 RX4
	}
	if (input_sel == MOBO_SRC_TOS1) {
		wm8805_write_byte(0x08, 0x36);	// 7:0 CLK2, 6:0 auto error handling enable, 5:1 zeros@error, 4:1 CLKOUT enable, 3:0 CLK1 out, 2-0:6 RX6
	}
 	else if (input_sel == MOBO_SRC_SPDIF) {
		wm8805_write_byte(0x08, 0x35);	// 7:0 CLK2, 6:0 auto error handling enable, 5:1 zeros@error, 4:1 CLKOUT enable, 3:0 CLK1 out, 2-0:5 RX5
 	}

	wm8805_write_byte(0x1E, 0x04);		// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:0 RX, 0:0 PLL,
	vTaskDelay(600);					// Allow for stability. 500 gives much better performance than 200.
}

// Delays of:
// 400 / 1200	Poor cold start
// 600 / 1200	Warm start OK. Cold start failed
// 600 / 3000	Warm start OK. Cold start OK
// 600 / 2000	Warm start OK. Cold start OK
// 600 / 1500	Warm start OK. Cold start OK


// Select PLL setting of the WM8805

void wm8805_pll(void) {
	uint8_t pll_sel = WM8805_PLL_NONE;

	wm8805_status.frequency = wm8805_srd();
	if (  ( (wm8805_status.frequency == FREQ_192) && (wm8805_status.pllmode != WM8805_PLL_192) ) ||
		  ( (wm8805_status.frequency != FREQ_192) && (wm8805_status.pllmode != WM8805_PLL_NORMAL) ) ) {
		if (wm8805_status.frequency == FREQ_192)
			wm8805_status.pllmode = WM8805_PLL_192;
		else
			wm8805_status.pllmode = WM8805_PLL_NORMAL;

		pll_sel = wm8805_status.pllmode;
	}

	// NO need for new PLL setup
	if (pll_sel == WM8805_PLL_NONE)
		return;

	// PLL setup has changed
	wm8805_write_byte(0x1E, 0x06);		// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:1 _RX, 0:0 PLL,

	// Default PLL setup for 44.1, 48, 88.2, 96, 176.4
	if (pll_sel == WM8805_PLL_NORMAL) {
		print_dbg_char('_');

		wm8805_write_byte(0x03, 0x21);	// PLL_K[7:0] 21
		wm8805_write_byte(0x04, 0xFD);	// PLL_K[15:8] FD
		wm8805_write_byte(0x05, 0x36);	// 7:0 , 6:0, 5-0:PLL_K[21:16] 36
		wm8805_write_byte(0x06, 0x07);	// 7:0 , 6:0 , 5:0 , 4:0 Prescale/1 , 3-2:PLL_N[3:0] 7
	}

	// Special PLL setup for 192
	else if (pll_sel == WM8805_PLL_192) {	// PLL setting 8.192
		print_dbg_char(169);			// High line

		wm8805_write_byte(0x03, 0xBA);	// PLL_K[7:0] BA
		wm8805_write_byte(0x04, 0x49);	// PLL_K[15:8] 49
		wm8805_write_byte(0x05, 0x0C);	// 7:0,  6:0, 5-0:PLL_K[21:16] 0C
		wm8805_write_byte(0x06, 0x08);	// 7: , 6: , 5: , 4: , 3-2:PLL_N[3:0] 8
	}

/* // Bad news: unified PLL setting doesn't work!
	else if (pll_sel == WM8805_PLL_EXP) { 	// Experimental PLL setting 8.0247 failed 192, 8.1 failed 176 and 192. Forget it!
		wm8805_write_byte(0x03, 0x66);	// PLL_K[7:0]
		wm8805_write_byte(0x04, 0x66);	// PLL_K[15:8]
		wm8805_write_byte(0x05, 0x06);	// 7:0,  6:0, 5-0:PLL_K[21:16]
		wm8805_write_byte(0x06, 0x08);	// 7: , 6: , 5: , 4: , 3-2:PLL_N[3:0] 8
	}
*/
	wm8805_write_byte(0x1E, 0x04);		// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:0 RX, 0:0 PLL,

	vTaskDelay(1500);	//500, 1000  bad start 3000, 2000, 1500 good start				// Let WM8805 PLL try to settle for some time (300-ish ms) FIX: too long?
}


// Set up WM8805 CLKOUTDIV so that CLKOUT is in the 22-24MHz range
void wm8805_clkdiv(void) {
	uint8_t temp;
	temp = wm8805_read_byte(0x0C);		// Read SPDSTAT
	temp = temp & 0x30;					// Consider bits 5-4

	if ( (temp == 0x20) || (temp == 0x30) )	// 44.1, 48, or 32
		wm8805_write_byte(0x07, 0x0C);	// 7:0 , 6:0, 5-4:MCLK=512fs , 3:1 MCLKDIV=1 , 2:1 FRACEN , 1-0:0
	else if (temp == 0x10)				// 88.2 or 96
		wm8805_write_byte(0x07, 0x1C);	// 7:0 , 6:0, 5-4:MCLK=256fs , 3:1 MCLKDIV=1 , 2:1 FRACEN , 1-0:0
	else								// 176.4 or 192
		wm8805_write_byte(0x07, 0x2C);	// 7:0 , 6:0, 5-4:MCLK=128fs , 3:1 MCLKDIV=1 , 2:1 FRACEN , 1-0:0
}


// Mute the WM8805 output
void wm8805_mute(void) {
//	print_dbg_char('M');

	// Empty outgoing buffers if owned by WM8805 code
	if ( (input_select == MOBO_SRC_SPDIF) || (input_select == MOBO_SRC_TOS2) || (input_select == MOBO_SRC_TOS1) ) {
		mobo_clear_dac_channel();
	}


	#ifdef HW_GEN_DIN20								// Dedicated mute pin, leaves clocks etc intact
		mobo_i2s_enable(MOBO_I2S_DISABLE);			// Hard-mute of I2S pin, try to avoid using this hardware!
	#endif
	dac_must_clear = DAC_MUST_CLEAR;				// Instruct uacX_device_audio_task.c to clear ouggoing DAC data

	mobo_xo_select(current_freq.frequency, MOBO_SRC_UAC2);	// Same functionality for both UAC sources
}


// Un-mute the WM8805
void wm8805_unmute(void) {
//	print_dbg_char('U');

	mobo_xo_select(wm8805_status.frequency, input_select);	// Outgoing I2S XO selector (and legacy MUX control)
	mobo_led_select(wm8805_status.frequency, input_select);	// User interface channel indicator
	mobo_clock_division(wm8805_status.frequency);			// Outgoing I2S clock division selector

	AK5394A_pdca_rx_enable(wm8805_status.frequency);		// New code to test for L/R swap


	ADC_buf_USB_IN = -1;							// Force init of MCU's ADC DMA port. Until this point it is NOT detecting zeros..

	#ifdef HW_GEN_DIN20
		mobo_i2s_enable(MOBO_I2S_ENABLE);			// Hard-unmute of I2S pin. NB: we should qualify outgoing data as 0 or valid music!!
	#endif
}


// Write a single byte to WM8805
uint8_t wm8805_write_byte(uint8_t int_adr, uint8_t int_data) {
    uint8_t dev_data[2];
    uint8_t status;

	dev_data[0] = int_adr;
	dev_data[1] = int_data;

	status = twi_write_out(WM8805_DEV_ADR, dev_data, 2);
	return status;
}


// Read a single byte from WM8805
uint8_t wm8805_read_byte(uint8_t int_adr) {
	uint8_t dev_data[1];

	dev_data[0] = int_adr;
	twi_write_out(WM8805_DEV_ADR, dev_data, 1);
	twi_read_in(WM8805_DEV_ADR, dev_data, 1);
	return dev_data[0];
}


// Wrapper test code
uint32_t wm8805_srd(void) {
	U32 freq = FREQ_44;
	U32 freq_prev;
	int i;

	i = 0;
	freq_prev = FREQ_INVALID;
	// 5 can fail
	// 10 seems solid but hard to say with only Juli@ as source 1 failure in N...
	// 14 1/20 failed to detect 96
	// 20 1/35 failed to detect 176.4
	// 30 1/26 failed to detect 96, seen as prev. song, wait longer to try to determine sample rate?
	// Even 400 will fail some times!

	while ( (i < 4) || ( (freq == FREQ_TIMEOUT) && (i < 80) ) ) {
		freq = wm8805_srd_asm2();
//		print_dbg_char('g');
		vTaskDelay(10);
		if ( (freq == freq_prev) && (freq != FREQ_TIMEOUT) )
			i++;
		freq_prev = freq;
	}


	if (freq == FREQ_44)
		print_dbg_char('1');
	else if (freq == FREQ_48)
		print_dbg_char('2');
	else if (freq == FREQ_88)
		print_dbg_char('3');
	else if (freq == FREQ_96)
		print_dbg_char('4');
	else if (freq == FREQ_176)
		print_dbg_char('5');
	else if (freq == FREQ_192)
		print_dbg_char('6');
	else
		print_dbg_char('U');

	return freq;
}

// Sample rate detection test
// This is MCU assembly code which replaces the non-functional sample rate detector inside the WM8805.
// It uses the same code for 44.1 and 48, and for 88.2 and 96. 176.4 and 192 are messed up too.
// The WM8805 sample rate change interrupt is based on its faulty detector and can't be trusted either.
// Todo: Make the pin to poll a parameter to the function rather than hard-coded.
//
// Compile with something like this:
// http://www.delorie.com/djgpp/v2faq/faq8_20.html
// gives:
// avr32-gcc -DFEATURE_BOARD_DEFAULT=feature_board_usbi2s -DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio -DFEATURE_IN_DEFAULT=feature_in_normal -DFEATURE_OUT_DEFAULT=feature_out_normal -DFEATURE_ADC_DEFAULT=feature_adc_none -DFEATURE_DAC_DEFAULT=feature_dac_generic -DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 -DFEATURE_LOG_DEFAULT=feature_log_500ms -DFEATURE_FILTER_DEFAULT=feature_filter_fir -DFEATURE_QUIRK_DEFAULT=feature_quirk_none -DUSB_STATE_MACHINE_DEBUG -DHW_GEN_DIN10 -DFEATURE_PRODUCT_AB1x -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -c -g -O2 -Wa,-a,-ad srd_test.c > srd_test.lst
//
// Alternatively:
// avr32-gcc -DFEATURE_BOARD_DEFAULT=feature_board_usbi2s -DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio -DFEATURE_IN_DEFAULT=feature_in_normal -DFEATURE_OUT_DEFAULT=feature_out_normal -DFEATURE_ADC_DEFAULT=feature_adc_none -DFEATURE_DAC_DEFAULT=feature_dac_generic -DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 -DFEATURE_LOG_DEFAULT=feature_log_500ms -DFEATURE_FILTER_DEFAULT=feature_filter_fir -DFEATURE_QUIRK_DEFAULT=feature_quirk_none -DUSB_STATE_MACHINE_DEBUG -DHW_GEN_DIN10 -DFEATURE_PRODUCT_AB1x -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -S -fverbose-asm -g -O2 srd_test.c
// avr32-gcc -DFEATURE_BOARD_DEFAULT=feature_board_usbi2s -DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio -DFEATURE_IN_DEFAULT=feature_in_normal -DFEATURE_OUT_DEFAULT=feature_out_normal -DFEATURE_ADC_DEFAULT=feature_adc_none -DFEATURE_DAC_DEFAULT=feature_dac_generic -DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 -DFEATURE_LOG_DEFAULT=feature_log_500ms -DFEATURE_FILTER_DEFAULT=feature_filter_fir -DFEATURE_QUIRK_DEFAULT=feature_quirk_none -DUSB_STATE_MACHINE_DEBUG -DHW_GEN_DIN10 -DFEATURE_PRODUCT_AB1x -DBOARD=SDRwdgtLite -DFREERTOS_USED -I../src/SOFTWARE_FRAMEWORK/DRIVERS/SSC/I2S -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PDCA -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TWIM -I../src/SOFTWARE_FRAMEWORK/UTILS/DEBUG -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/AUDIO -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/CDC -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/portable/GCC/AVR32_UC3 -I../src/SOFTWARE_FRAMEWORK/SERVICES/FREERTOS/Source/include -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB/CLASS/HID -I../src/SOFTWARE_FRAMEWORK/SERVICES/USB -I../src/CONFIG -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM/DEVICE -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB/ENUM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USBB -I../src/SOFTWARE_FRAMEWORK/DRIVERS/USART -I../src/SOFTWARE_FRAMEWORK/DRIVERS/TC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/WDT -I../src/SOFTWARE_FRAMEWORK/DRIVERS/CPU/CYCLE_COUNTER -I../src/SOFTWARE_FRAMEWORK/DRIVERS/EIC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/RTC -I../src/SOFTWARE_FRAMEWORK/DRIVERS/PM -I../src/SOFTWARE_FRAMEWORK/DRIVERS/GPIO -I../src/SOFTWARE_FRAMEWORK/DRIVERS/FLASHC -I../src/SOFTWARE_FRAMEWORK/UTILS/LIBS/NEWLIB_ADDONS/INCLUDE -I../src/SOFTWARE_FRAMEWORK/UTILS/PREPROCESSOR -I../src/SOFTWARE_FRAMEWORK/UTILS -I../src/SOFTWARE_FRAMEWORK/DRIVERS/INTC -I../src/SOFTWARE_FRAMEWORK/BOARDS -I../src -O2 -fdata-sections -Wall -c -fmessage-length=0 -mpart=uc3a3256 -ffunction-sections -masm-addr-pseudos -MMD -S -g -O2 srd_test.c
//
// A good asm syntax list:
// http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
//
// Test code for learning the asm code:
/*
#include "gpio.h"
#include <avr32/io.h>
#include "compiler.h"
#define GPIO  AVR32_GPIO
int foo(void) {
	#define TIMEOUT_LIM 8000;
	int timeout = 8000;
	// Code to determine GPIO constants, rewrite this (2 positions!) first, compile this section, then modify asm
	volatile avr32_gpio_port_t *gpio_port = &GPIO.port[AVR32_PIN_PA04 >> 5];
	while ( (timeout != 0) && ( ((gpio_port->pvr >> (AVR32_PIN_PA04 & 0x1F)) & 1) == 0) ) {
		timeout --;
	}
	return timeout;
}
*/
uint32_t wm8805_srd_asm2(void) {
	uint32_t timeout;

	// see srd_test.c and srd_test.lst

	// New board: Will move to PX09, pin 49

	// Determining speed at TP16 / DAC_0P / PA04 for now. Recompile prototype c to change io pin!
	// Test is done for up to 1 half period, then 2 full periods

	// HW_GEN_DIN10 gets patched to become like HW_GEN_DIN20 in this repect

	gpio_enable_gpio_pin(AVR32_PIN_PX09);	// Enable GPIO pin, not special IO (also for input). Needed?

	asm volatile(
//		"ssrf	16				\n\t"	// Disable global interrupt
		"mov	%0, 	2000	\n\t"	// Load timeout
		"mov	r9,		-61184	\n\t"	// Immediate load, set up pointer to PX09, recompile C for other IO pin, do once

		// If bit is 0, branch to loop while 0. If bit was 1, continue to loop while 1
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX09 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	28		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	S3				\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)

		// Wait while bit is 1, then count two half periods
		"S0:					\n\t"	// Loop while PA04 is 1
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX09 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	28		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	S0_done			\n\t"	// Branch if %0 bit 11 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S0				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S0_done:				\n\t"

		"mfsr	r10, 264		\n\t"	// Load 1st cycle counter into r10

		"S1:					\n\t"	// Loop while PA04 is 0
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX09 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	28		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	S1_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S1				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S1_done:				\n\t"

		"S2:					\n\t"	// Loop while PBA04 is 1
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX09 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	28		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	S2_done			\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S2				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S2_done:				\n\t"
		"rjmp	SRETURN__		\n\t"



		// Wait while bit is 0, then count two half periods
		"S3:					\n\t"	// Loop while PA04 is 0
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX09 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	28		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	S3_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S3				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S3_done:				\n\t"

		"mfsr	r10, 264		\n\t"	// Load 1st cycle counter into r10

		"S4:					\n\t"	// Loop while PBA04 is 1
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX09 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	28		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"brne	S4_done			\n\t"	// Branch if %0 bit 4 was 0 (bit was 0, Z becomes 0 i.e. not equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S4				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S4_done:				\n\t"

		"S5:					\n\t"	// Loop while PA04 is 0
		"ld.w	r8, 	r9[96]	\n\t"	// Load PX09 (and surroundings?) into r8, 		recompile C for other IO pin
		"bld	r8, 	28		\n\t"	// Bit load to Z and C, similar to above line,	recompile c for other IO pin
		"breq	S5_done			\n\t"	// Branch if %0 bit 4 was 1 (bit was 1, Z becomes 1 i.e. equal)
		"sub	%0,	1			\n\t"	// Count down
		"brne	S5				\n\t"	// Not done counting down
		"rjmp	SCOUNTD			\n\t"	// Countdown reached
		"S5_done:				\n\t"
		"rjmp	SRETURN__		\n\t"


		"SRETURN__:				\n\t"

		"mfsr	%0, 264			\n\t"	// Load 2nd cycle counter into r11
		"sub	%0, r10			\n\t"	// Return difference from 1st to 2nd cycle counter


		"SCOUNTD:				\n\t"	// Countdown reached, %0 is 0

//		"csrf	16				\n\t"	// Enable global interrupt
		:	"=r" (timeout)				// One output register
		:								// No input registers
		:	"r8", "r9", "r10"			// Clobber registers, pushed/popped unless assigned by GCC as temps
	);

//	timeout = 150 - timeout;

	// It looks like we have approx. With 66MHz CPU clock it looks like 1 ticks
	// Results from measurements and math:
	//  44.1 1478-1580 (1496.6)
	//  48.0 1358-1452 (1375.0)
	//	88.2  739- 790 ( 748.3)
	//  96.0  679- 726 ( 687.5)
	// 176.4  369- 396 ( 374.2)
	// 192.0  339- 363 ( 343.8)

	#define SLIM_44_LOW		1478
	#define SLIM_44_HIGH	1580 		// Gives timeout of 2000
	#define SLIM_48_LOW		1358
	#define SLIM_48_HIGH	1452
	#define SLIM_88_LOW		739
	#define SLIM_88_HIGH	790
	#define SLIM_96_LOW		679
	#define SLIM_96_HIGH	726
	#define SLIM_176_LOW	369		// Add margin??
	#define SLIM_176_HIGH	396
	#define SLIM_192_LOW	339
	#define SLIM_192_HIGH	363

	if ( (timeout >= SLIM_44_LOW) && (timeout <= SLIM_44_HIGH) ) {
//		print_dbg_char('1');
		return FREQ_44;
	}
	if ( (timeout >= SLIM_48_LOW) && (timeout <= SLIM_48_HIGH) ) {
//		print_dbg_char('2');
		return FREQ_48;
	}
	if ( (timeout >= SLIM_88_LOW) && (timeout <= SLIM_88_HIGH) ) {
//		print_dbg_char('3');
		return FREQ_88;
	}
	if ( (timeout >= SLIM_96_LOW) && (timeout <= SLIM_96_HIGH) ) {
//		print_dbg_char('4');
		return FREQ_96;
	}
	if ( (timeout >= SLIM_176_LOW) && (timeout <= SLIM_176_HIGH) ) {
//		print_dbg_char('6');
		return FREQ_176;
	}
	if ( (timeout >= SLIM_192_LOW) && (timeout <= SLIM_192_HIGH) ) {
//		print_dbg_char('6');
		return FREQ_192;
	}
	else {
//		print_dbg_char('F');
		return FREQ_TIMEOUT;	// Every uncertainty treated as timeout...
	}

}


#endif  // HW_GEN_DIN10 or HW_GEN_DIN20
