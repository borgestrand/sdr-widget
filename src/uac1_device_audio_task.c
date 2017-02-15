/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Management of the USB device Audio task.
 *
 * This file manages the USB device Audio task.
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

//_____  I N C L U D E S ___________________________________________________

#include <stdio.h>
#include "usart.h"     // Shall be included before FreeRTOS header files, since 'inline' is defined to ''; leading to
                       // link errors
#include "conf_usb.h"


#if USB_DEVICE_FEATURE == ENABLED

#include "board.h"
#include "features.h"
#ifdef FREERTOS_USED
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "gpio.h"
#include "pdca.h"
#include "usb_drv.h"
#include "usb_descriptors.h"
#include "uac1_usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "uac1_usb_specific_request.h"
#include "device_audio_task.h"
#include "uac1_device_audio_task.h"

#if LCD_DISPLAY            // Multi-line LCD display
#include "taskLCD.h"
#endif

#include "composite_widget.h"
#include "taskAK5394A.h"

// To access input select constants
#include "Mobo_config.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________


// #define FB_RATE_DELTA (1<<12)
#define FB_RATE_DELTA 64 // BSB 20130603 stability??
#define FB_RATE_DELTA_NUM 2

// BSB 20130605 FB_rate calculation with 2 levels, imported from UAC2 code
#define SPK1_GAP_USKIP DAC_BUFFER_SIZE * 7 / 4	// Almost a full buffer up in distance => enable skip/insert
#define SPK1_GAP_U2    DAC_BUFFER_SIZE * 6 / 4	// A half buffer up in distance	=> Speed up host a lot
#define	SPK1_GAP_U1    DAC_BUFFER_SIZE * 5 / 4	// A quarter buffer up in distance => Speed up host a bit
#define SPK1_GAP_NOM   DAC_BUFFER_SIZE * 4 / 4	// Ideal distance is half the size of linear buffer
#define SPK1_GAP_L1	   DAC_BUFFER_SIZE * 3 / 4  // A quarter buffer down in distance => Slow down host a bit
#define SPK1_GAP_L2	   DAC_BUFFER_SIZE * 2 / 4  // A half buffer down in distance => Slow down host a lot
#define SPK1_GAP_LSKIP DAC_BUFFER_SIZE * 1 / 4	// Almost a full buffer down in distance => enable skip/insert
#define	SPK1_PACKETS_PER_GAP_CALCULATION 8		// This is UAC1 which counts in ms. Gap calculation every 8ms, EP reporting every 32
#define	SPK1_PACKETS_PER_GAP_SKIP 1				// After a skip/insert, recalculate gap immediately, then again after 1ms
#define SPK1_HOST_FB_DEAD_AFTER 200				// How many audio packets may arrive without host polling feedback, before we declare FB dead?
#define SPK1_SKIP_EN_GAP 1                      // Enable skip/insert due to low gap
#define SPK1_SKIP_EN_DEAD 2						// Enable skip/insert due to dead host feedback system
#define SPK1_SKIP_LIMIT_14 2<<14 			    // 10.14 and 12.14 format. |accumulated error| must be > 2 samples.

//_____ D E C L A R A T I O N S ____________________________________________


//? why are these defined as statics?

static U32  index, spk_index;
static U16  old_gap = DAC_BUFFER_SIZE;
// static U8 ADC_buf_USB_IN, DAC_buf_USB_OUT;		// These are now global the ID number of the buffer used for sending out to the USB
static volatile U32 *audio_buffer_ptr;
//static volatile U32 *spk_buffer_ptr;

static U8 ep_audio_in, ep_audio_out, ep_audio_out_fb;

//!
//! @brief This function initializes the hardware/software resources
//! required for device Audio task.
//!
void uac1_device_audio_task_init(U8 ep_in, U8 ep_out, U8 ep_out_fb)
{
	index     =0;
	ADC_buf_USB_IN = 0;
	audio_buffer_ptr = audio_buffer_0;
	spk_index = 0;
	DAC_buf_USB_OUT = 0;
//	spk_buffer_ptr = spk_buffer_0;
	mute = FALSE;
	spk_mute = FALSE;

	// With working volume flash
	// spk_vol_usb_L = usb_volume_flash(CH_LEFT, 0, VOL_READ);		// Fetch stored or default volume setting
	// spk_vol_usb_R = usb_volume_flash(CH_RIGHT, 0, VOL_READ);
	// Without working volume flash, spk_vol_usb_? = VOL_DEFAULT is set in device_audio_task.c
	spk_vol_mult_L = usb_volume_format(spk_vol_usb_L);
	spk_vol_mult_R = usb_volume_format(spk_vol_usb_R);

	ep_audio_in = ep_in;
	ep_audio_out = ep_out;
	ep_audio_out_fb = ep_out_fb;

	xTaskCreate(uac1_device_audio_task,
				configTSK_USB_DAUDIO_NAME,
				configTSK_USB_DAUDIO_STACK_SIZE,
				NULL,
				configTSK_USB_DAUDIO_PRIORITY,
				NULL);
}

//!
//! @brief Entry point of the device Audio task management
//!

void uac1_device_audio_task(void *pvParameters)
{
	Bool playerStarted = FALSE; // BSB 20150516: changed into global variable
	static U32  time=0;
	static Bool startup=TRUE;
	int i;
//	int delta_num = 0;
	U16 num_samples, num_remaining, gap;
	S16 time_to_calculate_gap = 0; // BSB 20131101 New variables for skip/insert
	U16 packets_since_feedback = 0;
	U8 skip_enable = 0;
	U8 skip_indicate = 0;	// Should we show skipping on module LEDs?
	U16 samples_to_transfer_OUT = 1; // Default value 1. Skip:0. Insert:2
	S32 FB_error_acc = 0;	// BSB 20131102 Accumulated error for skip/insert
	U8 sample_HSB;
	U8 sample_MSB;
	U8 sample_SB;
	U8 sample_LSB;
//	U8 toggle_07 = 0;	// BSB 20131206 keep track of GPIO_07 / PX31
	S32 sample_L = 0;
	S32 sample_R = 0; // BSB 20131102 Expanded for skip/insert, 20160322 changed to S32!
	const U8 EP_AUDIO_IN = ep_audio_in;
	const U8 EP_AUDIO_OUT = ep_audio_out;
	const U8 EP_AUDIO_OUT_FB = ep_audio_out_fb;
	const U8 IN_LEFT = FEATURE_IN_NORMAL ? 0 : 1;
	const U8 IN_RIGHT = FEATURE_IN_NORMAL ? 1 : 0;
	const U8 OUT_LEFT = FEATURE_OUT_NORMAL ? 0 : 1;
	const U8 OUT_RIGHT = FEATURE_OUT_NORMAL ? 1 : 0;
	volatile avr32_pdca_channel_t *pdca_channel = pdca_get_handler(PDCA_CHANNEL_SSC_RX);
	volatile avr32_pdca_channel_t *spk_pdca_channel = pdca_get_handler(PDCA_CHANNEL_SSC_TX);
	uint32_t silence_USB = SILENCE_USB_LIMIT;	// BSB 20150621: detect silence in USB channel, initially assume silence
	uint32_t silence_det = 0;

	// BSB 20130602: code section moved to uac1_usb_specific_request.c
	// if (current_freq.frequency == FREQ_48) FB_rate = 48 << 14;
	// else FB_rate = (44 << 14) + (1 << 14)/10;


	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	while (TRUE) {
		vTaskDelayUntil(&xLastWakeTime, UAC1_configTSK_USB_DAUDIO_PERIOD);

#ifdef HW_GEN_DIN20
		// A detected usb swap must be acknowledged
//		if ( (!Is_device_enumerated()) && (usb_ch_swap != USB_CH_SWAPDET) ) { time=0; startup=TRUE; continue; };
		if (   ( (input_select == MOBO_SRC_UAC1) || (input_select == MOBO_SRC_NONE) )
			&& (!Is_device_enumerated())
			&& (usb_ch_swap != USB_CH_SWAPDET) ) {
				time=0;
				startup=TRUE;
				continue;
		}
#elif (defined HW_GEN_DIN10)
		if (   ( (input_select == MOBO_SRC_UAC1) || (input_select == MOBO_SRC_NONE) )
			&& (!Is_device_enumerated())   ) {
				time=0;
				startup=TRUE;
				continue;
		}
#else
		// First, check the device enumeration state
		if (!Is_device_enumerated()) { time=0; startup=TRUE; continue; };
#endif

		if( startup ) {
			time+=UAC1_configTSK_USB_DAUDIO_PERIOD;
#define STARTUP_LED_DELAY  10000
			if ( time<= 1*STARTUP_LED_DELAY ) {
				LED_On( LED0 );

				#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)
					// pdca disable code must be moved to something dealing with spdif playback use of ADC interface
				#else
					pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
					pdca_disable(PDCA_CHANNEL_SSC_RX);
				#endif

				//	            LED_On( LED1 );
			} else if( time== 2*STARTUP_LED_DELAY ) LED_On( LED1 );
			else if( time== 3*STARTUP_LED_DELAY ) LED_On( LED2 );
			else if( time== 4*STARTUP_LED_DELAY ) LED_On( LED3 );
			else if( time== 5*STARTUP_LED_DELAY ) LED_Off( LED0 );
			else if( time== 6*STARTUP_LED_DELAY ) LED_Off( LED1 );
			else if( time== 7*STARTUP_LED_DELAY ) LED_Off( LED2 );
			else if( time== 8*STARTUP_LED_DELAY ) LED_Off( LED3 );
			else if( time >= 9*STARTUP_LED_DELAY ) {
				startup=FALSE;

				ADC_buf_DMA_write = 0;
				ADC_buf_USB_IN = 0;
				DAC_buf_USB_OUT = 0;
				DAC_buf_DMA_read = 0; // Only place outside taskAK53984A.c where DAC_buf_DMA_read is written!
				index = 0;


				#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)
				#else
					if (!FEATURE_ADC_NONE) {
						// Wait for the next frame synchronization event
						// to avoid channel inversion.  Start with left channel - FS goes low
						while (!gpio_get_pin_value(AK5394_LRCK));
						while (gpio_get_pin_value(AK5394_LRCK));

						// Enable now the transfer.
						pdca_enable(PDCA_CHANNEL_SSC_RX);
						pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
					}
				#endif

			}
		}

// Seriously messing with ADC interface...
// Rewriting for polling the consumer, not the manufacturer.
// We can expect consumer (MCU's DAC interface) to be steady and under firmware control,
// .. while producer (MCU's ADC interface and WM8805 I2S out) can fall out at any time.
#if ((defined HW_GEN_DIN10) || (defined HW_GEN_DIN20))
		static int DAC_buf_DMA_read_local = -1;
		int DAC_buf_DMA_read_temp;

		/*
		// Startup condition with ADC_buf_DMA_write_local == -1 resets spk_index according to DAC_buf_USB_OUT
		// How could this even work in UAC1??
		if (DAC_buf_DMA_read_local == -1) {
			num_remaining = pdca_channel->tcr;
			ADC_buf_USB_IN = ADC_buf_DMA_write; // ..USB_IN is wrong in this context, it's into the MCU for forwarding to the DAC
			index = ADC_BUFFER_SIZE - num_remaining;
			index = index & ~((U32)1); 	// Clear LSB in order to start with L sample
			DAC_buf_DMA_read_local = 2; // Done initiating. Must improve init code!
		}
*/

		if (ADC_buf_USB_IN == -1) {
			pdca_enable(PDCA_CHANNEL_SSC_RX);	// Enable I2S reception at MCU's ADC port. FIX: Also do this at sample rate chg?
			num_remaining = pdca_channel->tcr;
			ADC_buf_USB_IN = ADC_buf_DMA_write; // ..USB_IN is wrong in this context, it's into the MCU for forwarding to the DAC
			index = ADC_BUFFER_SIZE - num_remaining;
			index = index & ~((U32)1); 	// Clear LSB in order to start with L sample
//			DAC_buf_DMA_read_local = 2; // Done initiating
		}



		if ( ( (input_select != MOBO_SRC_UAC1) && (input_select != MOBO_SRC_NONE) ) ) {
			DAC_buf_DMA_read_temp = DAC_buf_DMA_read; // Interrupt may strike at any time, cache the buffer selector

			if (DAC_buf_DMA_read_temp != DAC_buf_DMA_read_local) { // Must transfer previous half-ring-buffer
				DAC_buf_DMA_read_local = DAC_buf_DMA_read_temp;

				if (DAC_buf_DMA_read_temp == 1)
					gpio_set_gpio_pin(AVR32_PIN_PX18);			// Pin 84
				else
					gpio_clr_gpio_pin(AVR32_PIN_PX18);			// Pin 84

				for( i=0 ; i < DAC_BUFFER_SIZE ; i+=2 ) {
					// Input from audio_buffer
					if (ADC_buf_USB_IN == 0) {
						sample_L = audio_buffer_0[index+IN_LEFT];
						sample_R = audio_buffer_0[index+IN_RIGHT];
					}
					else if (ADC_buf_USB_IN == 1) {
						sample_L = audio_buffer_1[index+IN_LEFT];
						sample_R = audio_buffer_1[index+IN_RIGHT];
					}

					// Manually increase counter in audio_buffer
					index += 2;
					if (index >= ADC_BUFFER_SIZE) {
						index = 0;
						ADC_buf_USB_IN = 1 - ADC_buf_USB_IN;

#ifdef USB_STATE_MACHINE_DEBUG
						if (ADC_buf_USB_IN == 1)
							gpio_set_gpio_pin(AVR32_PIN_PX30);
						else
							gpio_clr_gpio_pin(AVR32_PIN_PX30);
#endif
					}


					// Output to one full spk_buffer (half of ring buffer)
					// Automatically increase counter in spk_buffer (for loop)
					if (DAC_buf_DMA_read_temp == 0) {
						spk_buffer_0[i+OUT_LEFT] = sample_L;
						spk_buffer_0[i+OUT_RIGHT] = sample_R;
					}
					else if (DAC_buf_DMA_read_temp == 1) {
						spk_buffer_1[i+OUT_LEFT] = sample_L;
						spk_buffer_1[i+OUT_RIGHT] = sample_R;
					}

				} // for DAC_BUFFER_SIZE
			} // DAC_buf_DMA_read toggle
		} // input select
#endif

// Done messing with ADC interface




// Should we remove old ADC code from here?
		num_samples = 48;
			if (usb_alternate_setting == 1) {

				if (Is_usb_in_ready(EP_AUDIO_IN)) {	// Endpoint buffer free ?
					Usb_ack_in_ready(EP_AUDIO_IN);	// acknowledge in ready

					// Sync AK data stream with USB data stream
					// AK data is being filled into ~ADC_buf_DMA_write, ie if ADC_buf_DMA_write is 0
					// buffer 0 is set in the reload register of the pdca
					// So the actual loading is occuring in buffer 1
					// USB data is being taken from ADC_buf_USB_IN

					// find out the current status of PDCA transfer
					// gap is how far the ADC_buf_USB_IN is from overlapping ADC_buf_DMA_write

					num_remaining = pdca_channel->tcr;
					if (ADC_buf_DMA_write != ADC_buf_USB_IN) {
						// AK and USB using same buffer
						if ( index < (ADC_BUFFER_SIZE - num_remaining)) gap = ADC_BUFFER_SIZE - num_remaining - index;
						else gap = ADC_BUFFER_SIZE - index + ADC_BUFFER_SIZE - num_remaining + ADC_BUFFER_SIZE;
					} else {
						// usb and pdca working on different buffers
						gap = (ADC_BUFFER_SIZE - index) + (ADC_BUFFER_SIZE - num_remaining);
					}


					// Sync the USB stream with the AK stream
					// throttle back
					if  (gap < ADC_BUFFER_SIZE/2) {
						num_samples--;
					} else {
						// speed up
						if  (gap > (ADC_BUFFER_SIZE + ADC_BUFFER_SIZE/2)) {
							num_samples++;
						}
					}

					Usb_reset_endpoint_fifo_access(EP_AUDIO_IN);
					for( i=0 ; i < num_samples ; i++ ) {
						// Fill endpoint with sample raw
						if (mute==FALSE) {
							if (ADC_buf_USB_IN == 0) {
								sample_LSB = audio_buffer_0[index+IN_LEFT];
								sample_SB = audio_buffer_0[index+IN_LEFT] >> 8;
								sample_MSB = audio_buffer_0[index+IN_LEFT] >> 16;
							} else {
								sample_LSB = audio_buffer_1[index+IN_LEFT];
								sample_SB = audio_buffer_1[index+IN_LEFT] >> 8;
								sample_MSB = audio_buffer_1[index+IN_LEFT] >> 16;
							}

							Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample_LSB);
							Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample_SB);
							Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample_MSB);

							if (ADC_buf_USB_IN == 0) {
								sample_LSB = audio_buffer_0[index+IN_RIGHT];
								sample_SB = audio_buffer_0[index+IN_RIGHT] >> 8;
								sample_MSB = audio_buffer_0[index+IN_RIGHT] >> 16;
							} else {
								sample_LSB = audio_buffer_1[index+IN_RIGHT];
								sample_SB = audio_buffer_1[index+IN_RIGHT] >> 8;
								sample_MSB = audio_buffer_1[index+IN_RIGHT] >> 16;
							}

							Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample_LSB);
							Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample_SB);
							Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample_MSB);

							index += 2;
							if (index >= ADC_BUFFER_SIZE) {
								index=0;
								ADC_buf_USB_IN = 1 - ADC_buf_USB_IN;
							}
						} else {
							Usb_write_endpoint_data(EP_AUDIO_IN, 8, 0x00);
							Usb_write_endpoint_data(EP_AUDIO_IN, 8, 0x00);
							Usb_write_endpoint_data(EP_AUDIO_IN, 8, 0x00);
							Usb_write_endpoint_data(EP_AUDIO_IN, 8, 0x00);
							Usb_write_endpoint_data(EP_AUDIO_IN, 8, 0x00);
							Usb_write_endpoint_data(EP_AUDIO_IN, 8, 0x00);

						}
					}
					Usb_send_in(EP_AUDIO_IN);		// send the current bank
				}
			} // end alt setting == 1

// Should we remove old ADC code to here?


#ifdef HW_GEN_DIN20
			if ( (usb_alternate_setting_out == 1) && (usb_ch_swap == USB_CH_NOSWAP) ) {
#else
			if (usb_alternate_setting_out == 1) {
#endif
				// BSB 20131031 actual gap calculation moved to after OUT data processing


				if (Is_usb_in_ready(EP_AUDIO_OUT_FB)) {	// Endpoint buffer free ?

/*
#ifdef USB_STATE_MACHINE_DEBUG						// BSB 20131206 Toggle GPIO_07 / TP72 to indicate FB EP poll
					if (toggle_07 == 1) {			//                     = PX31 = TP72
						gpio_clr_gpio_pin(AVR32_PIN_PX31);
						toggle_07 = 0;
					}
					else {
						gpio_set_gpio_pin(AVR32_PIN_PX31);
						toggle_07 = 1;
					}
#endif
*/

					Usb_ack_in_ready(EP_AUDIO_OUT_FB);	// acknowledge in ready
					Usb_reset_endpoint_fifo_access(EP_AUDIO_OUT_FB);

/* BSB 20131101
 * A "stupid" Host is able to read the feedback but does not consider it. Emulate that by resetting packets_since_feedback
 * and sending the Host the initial feedback value. Initial feedback is seeded with a hardcoded offset FB_INITIAL_OFFSET
 *
 * A "dead" Host is not reading the feedback. Emulate that by not resetting packets_since_feedbakc and sending the Host the
 * initial feedback value.
 */

					if (FEATURE_HDEAD_OFF)
						packets_since_feedback = 0;

					if (Is_usb_full_speed_mode()) {
						// FB rate is 3 bytes in 10.14 format
#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20) 	// With WM8805 input, USB subsystem will be running off a completely wacko MCLK!
						if ( (input_select != MOBO_SRC_UAC1) || (FEATURE_HSTUPID_ON) || (FEATURE_HDEAD_ON) ) {	// BSB 20131101
#else
						if ( (FEATURE_HSTUPID_ON) || (FEATURE_HDEAD_ON) ) {	// BSB 20131101
#endif
							sample_LSB = FB_rate_nominal;		// but not consider it. Emulate this by sending the nominal
							sample_SB = FB_rate_nominal >> 8;	// FB rate and not the one generated by the firmware's feedback
							sample_MSB = FB_rate_nominal >> 16;
						}
						else {									// Send firmware's feedback
							sample_LSB = FB_rate;
							sample_SB = FB_rate >> 8;
							sample_MSB = FB_rate >> 16;
						}
						Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_LSB);
						Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_SB);
						Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_MSB);
					}
					else {
						// HS mode - Not likely to ever be used in UAC1 - UNTESTED code!
						// FB rate is 4 bytes in 12.14 format
#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20) 	// With WM8805 input, USB subsystem will be running off a completely wacko MCLK!
						if ( (input_select != MOBO_SRC_UAC1) || (FEATURE_HSTUPID_ON) || (FEATURE_HDEAD_ON) ) {	// BSB 20131101
#else
						if ( (FEATURE_HSTUPID_ON) || (FEATURE_HDEAD_ON) ) {	// BSB 20131101
#endif
							sample_LSB = FB_rate_nominal;
							sample_SB = FB_rate_nominal >> 8;
							sample_MSB = FB_rate_nominal >> 16;
							sample_HSB = FB_rate_nominal >> 24;
						}
						else {
							sample_LSB = FB_rate;
							sample_SB = FB_rate >> 8;
							sample_MSB = FB_rate >> 16;
							sample_HSB = FB_rate >> 24;
						}
						Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_LSB);
						Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_SB);
						Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_MSB);
						Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_HSB);
					}

					Usb_send_in(EP_AUDIO_OUT_FB);
				} // end if (Is_usb_in_ready(EP_AUDIO_OUT_FB)) // Endpoint buffer free ?



				if (Is_usb_out_received(EP_AUDIO_OUT)) {
					spk_usb_heart_beat++;			// indicates EP_AUDIO_OUT receiving data from host

					Usb_reset_endpoint_fifo_access(EP_AUDIO_OUT);
					num_samples = Usb_byte_count(EP_AUDIO_OUT) / 6; // Hardcoded 24-bit mono samples, 6 bytes for stereo

					if( (!playerStarted) || (audio_OUT_must_sync) ) {	// BSB 20140917 attempting to help uacX_device_audio_task.c synchronize to DMA
						time_to_calculate_gap = 0;			// BSB 20131031 moved gap calculation for DAC use
						packets_since_feedback = 0;			// BSB 20131031 assuming feedback system may soon kick in
						FB_error_acc = 0;					// BSB 20131102 reset feedback error
						FB_rate = FB_rate_initial;			// BSB 20131113 reset feedback rate
						old_gap = DAC_BUFFER_SIZE;			// BSB 20131115 moved here
						skip_enable = 0;					// BSB 20131115 Not skipping yet...
						skip_indicate = 0;
						usb_buffer_toggle = 0;				// BSB 20131201 Attempting improved playerstarted detection

						// BSB 20150725: Buffer alignment takes place as soon as 1st nonzero sample is received.
						// FIX: Move above code as well?
					} // end if (!playerStarted) || (audio_OUT_must_sync)

					// BSB 20140917 attempting to help uacX_device_audio_task.c synchronize to DMA
					audio_OUT_alive = 1;					// Indicate samples arriving on audio OUT endpoint. Do this after syncing


					// Received samples in 10.14 or 12.14 format is num_samples * 1<<14
					// Error increases when Host (in average) sends too much data compared to FB_rate
					// A high error means we must skip.

/*					// Try to detect a dead Host feedback system
					if (FEATURE_NOSKIP_OFF) { 				// If skip/insert isn't disabled...
						if (packets_since_feedback > SPK1_HOST_FB_DEAD_AFTER)
							skip_enable |= SPK1_SKIP_EN_DEAD;	// Enable skip/insert due to dead host feedback system
						else {
							packets_since_feedback ++;
							skip_enable &= ~SPK1_SKIP_EN_DEAD;	// Disable skip/insert due to dead host feedback system
						}
					}
*/

					// Default:1 Skip:0 Insert:2 Only one skip or insert per USB package
					// .. prior to for(num_samples) Hence 1st sample in a package is skipped or inserted
					samples_to_transfer_OUT = 1;
					if (skip_enable == 0) {					// Respond to all skip enablers
						FB_error_acc = 0;
					}
					else {
						FB_error_acc = FB_error_acc + ((S32)num_samples * 1<<14) - FB_rate;
						if (FB_error_acc > SPK1_SKIP_LIMIT_14) {	// Must skip
							samples_to_transfer_OUT = 0;			// Do some skippin'
							FB_error_acc = FB_error_acc - (1<<14);
							time_to_calculate_gap = -1;				// Immediate gap re-calculation
							skip_indicate = 1;
							LED_On(LED0);							// Indicate skipping on module LED

#ifdef USB_STATE_MACHINE_DEBUG
							print_dbg_char('s');
#endif

						}
						else if (FB_error_acc < -SPK1_SKIP_LIMIT_14) {	// Must insert
							samples_to_transfer_OUT = 2;			// Do some insertin'
							FB_error_acc = FB_error_acc + (1<<14);
							time_to_calculate_gap = -1;				// Immediate gap re-calculation
							skip_indicate = 1;
							LED_On(LED1);							// Indicate skipping on module LED

#ifdef USB_STATE_MACHINE_DEBUG
							print_dbg_char('i');
#endif
						}
					}

// BSB 20131106 some notes on using AB-1.1 analog output for debug
//					sample_L = 0x007FFFFF; // posative 24-bit full scale for calibration AB-1.1: 2.744VDC NB!! ES9023 keep squares at least 3dB
//					sample_R = 0xFF800001; // negitive 24-bit full scale for calibration AB-1.2: -2.745VDC     below full-scale!
//					sample_L = (U32)num_samples << 16; // +127 is maximum. Expect 9 times 44 and once 45. Multimeter:945.4mVDC dead on!
//					sample_R = (U32)FB_rate_initial << 2; // It was <<'ed by 14, expect 44.1 /  Multimeter: 948.6mVDC not quite dead on..
//					sample_R = (U32)num_samples << 16; // +127 is maximum. Expect 9 times 44 and once 45. Multimeter: 948.0mVDC
//					sample_L = (U32)FB_rate_initial << 2; // It was <<'ed by 14, expect 44.1 /  Multimeter: 946.1mVDC dead on!
//					sample_L = (U32)FB_error_acc << 2;
//					sample_R = (U32)num_samples << 16;

					// ON this particular AB-1.2, the Left channel is more accurate at this particular measurement. We'll wait with further
					// calibration. FB_rate_initial is verified. Value of 1<<16 = 21mV

					silence_det = 0;						// We're looking for non-zero audio data..
					for (i = 0; i < num_samples; i++) {
						sample_LSB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						sample_SB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						sample_MSB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						sample_L = (((U32) sample_MSB) << 16) + (((U32)sample_SB) << 8) + sample_LSB;
						silence_det |= sample_L;

#ifdef FEATURE_VOLUME_CTRL
						if (spk_vol_mult_L != VOL_MULT_UNITY) {	// Only touch gain-controlled samples
							// 24-bit data words. First shift up to 32 bit. Do math and shift down
							sample_L <<= 8;
							sample_L = (S32)( (int64_t)( (int64_t)(sample_L) * (int64_t)spk_vol_mult_L ) >> VOL_MULT_SHIFT) ;
							sample_L += rand8(); // dither in bits 7:0, will this be optimized away due to next line?
							sample_L >>= 8;
						}
#endif
						sample_LSB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						sample_SB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						sample_MSB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						sample_R = (((U32) sample_MSB) << 16) + (((U32)sample_SB) << 8) + sample_LSB;
						silence_det |= sample_R;

#ifdef FEATURE_VOLUME_CTRL
						if (spk_vol_mult_R != VOL_MULT_UNITY) {	// Only touch gain-controlled samples
							// 24-bit data words. First shift up to 32 bit. Do math and shift down
							sample_R <<= 8;
							sample_R = (S32)( (int64_t)( (int64_t)(sample_R) * (int64_t)spk_vol_mult_R ) >> VOL_MULT_SHIFT) ;
							sample_R += rand8(); // dither in bits 7:0, will this be optimized away due to next line?
							sample_R >>= 8;
						}
#endif

						// New site for setting playerStarted and aligning buffers
						if ( (silence_det != 0) && (input_select == MOBO_SRC_NONE) ) {	// There is actual USB audio.
							#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)			// With WM8805 subsystem, handle semaphore
								#ifdef USB_STATE_MACHINE_DEBUG
									print_dbg_char('t');					// Debug semaphore, lowercase letters in USB tasks
									if (xSemaphoreTake(input_select_semphr, 0) == pdTRUE) {		// Re-take of taken semaphore returns false
										print_dbg_char('[');

										mobo_clock_division(current_freq.frequency);	// Re-configure correct USB sample rate

										input_select = MOBO_SRC_UAC1;
										#ifdef HW_GEN_DIN20
											mobo_i2s_enable(MOBO_I2S_ENABLE);		// Hard-unmute of I2S pin
										#endif
									}										// Hopefully, this code won't be called repeatedly. Would there be time??
									else
										print_dbg_char(']');
								#else // not debug
									if (xSemaphoreTake(input_select_semphr, 0) == pdTRUE)

										mobo_clock_division(current_freq.frequency);	// Re-configure correct USB sample rate

										input_select = MOBO_SRC_UAC1;
										#ifdef HW_GEN_DIN20
											mobo_i2s_enable(MOBO_I2S_ENABLE);		// Hard-unmute of I2S pin
										#endif
								#endif
							#else // not HW_GEN_DIN10/20											// No WM8805, take control
								input_select = MOBO_SRC_UAC1;
							#endif
						}

						// Do we own output (semaphore)? If so, change I2S setting and resync _once_
						// Why are we doing this within for num_samples loop?
						if ( (!playerStarted) && (input_select == MOBO_SRC_UAC1) ) {
							playerStarted = TRUE;					// Arrival of nonzero sample is now indication of playerStarted
	//						silence_USB = SILENCE_USB_INIT;			// Let loop code determine silence. FIX: test with sample rate changes!

							// FIX: mobo_xo_select will control I2S mux in GEN_DINx0. Verify semaphore mutex ownership!

		            		mobo_xo_select(current_freq.frequency, input_select);	// Give USB the I2S control with proper MCLK

	#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)			// With WM8805 subsystem set RGB front LED
							mobo_led_select(current_freq.frequency, input_select);
	#endif

							// Align buffers
							audio_OUT_must_sync = 0;				// BSB 20140917 attempting to help uacX_device_audio_task.c synchronize to DMA
// Stability?				Disable_global_interrupt();				// RTOS-atomic operation
//							portENTER_CRITICAL();
								num_remaining = spk_pdca_channel->tcr;
								DAC_buf_USB_OUT = DAC_buf_DMA_read;		// Keep resyncing until playerStarted becomes true
//							portEXIT_CRITICAL();
// Stability?				Enable_global_interrupt();
							LED_Off(LED0);							// The LEDs on the PCB near the MCU
							LED_Off(LED1);

#ifdef USB_STATE_MACHINE_DEBUG
							if (DAC_buf_USB_OUT == 1)					// Debug message 'p' removed along with #ifdefs
								gpio_set_gpio_pin(AVR32_PIN_PX30); 	// BSB 20140820 debug on GPIO_06/TP71 (was PX55 / GPIO_03)
							else
								gpio_clr_gpio_pin(AVR32_PIN_PX30); 	// BSB 20140820 debug on GPIO_06/TP71 (was PX55 / GPIO_03)
#endif

							spk_index = DAC_BUFFER_SIZE - num_remaining;
							spk_index = spk_index & ~((U32)1); 	// Clear LSB in order to start with L sample
						}

						// Semaphore not taken, or muted, output zeros
						if ( (input_select != MOBO_SRC_UAC1) || (spk_mute) ) {
							sample_L = 0;
							sample_R = 0;
						}

						// Only write to spk_buffer_? when allowed
						if ( (input_select == MOBO_SRC_UAC1) || (input_select == MOBO_SRC_NONE) ) {
							while (samples_to_transfer_OUT-- > 0) { // Default:1 Skip:0 Insert:2 Apply to 1st stereo sample in packet
								if (DAC_buf_USB_OUT == 0) {
									spk_buffer_0[spk_index+OUT_LEFT] = sample_L;
									spk_buffer_0[spk_index+OUT_RIGHT] = sample_R;
								}
								else {
									spk_buffer_1[spk_index+OUT_LEFT] = sample_L;
									spk_buffer_1[spk_index+OUT_RIGHT] = sample_R;
								}

								spk_index += 2;
								if (spk_index >= DAC_BUFFER_SIZE) {
									spk_index = 0;
									DAC_buf_USB_OUT = 1 - DAC_buf_USB_OUT;

#ifdef USB_STATE_MACHINE_DEBUG
									if (DAC_buf_USB_OUT == 1)
										gpio_set_gpio_pin(AVR32_PIN_PX30); // BSB 20140820 debug on GPIO_06/TP71 (was PX55 / GPIO_03)
									else
										gpio_clr_gpio_pin(AVR32_PIN_PX30); // BSB 20140820 debug on GPIO_06/TP71 (was PX55 / GPIO_03)
#endif

									// BSB 20131201 attempting improved playerstarted detection
									usb_buffer_toggle--;					// Counter is increased by DMA, decreased by seq. code
								}
							}
							samples_to_transfer_OUT = 1; // Revert to default:1. I.e. only one skip or insert per USB package
						}
					} // end for num_samples

					// Detect USB silence. We're counting USB packets. UAC2: 250us, UAC1: 1ms
					if (silence_det == 0) {
						if (!USB_IS_SILENT())
							silence_USB += 4;
					}
					else // stereo sample is non-zero
						silence_USB = SILENCE_USB_INIT;			// USB interface is not silent!

					Usb_ack_out_received_free(EP_AUDIO_OUT);

					if ( (USB_IS_SILENT()) && (input_select == MOBO_SRC_UAC1) ) { // Oops, we just went silent, probably from pause
						playerStarted = FALSE;

						#ifdef HW_GEN_DIN20						// Dedicated mute pin
							mobo_i2s_enable(MOBO_I2S_DISABLE);	// Hard-mute of I2S pin
						#endif

						// Clear buffers for good measure! That may offload uac1_AK5394A_task() ?? and present a good mute to WM8805
						for (i = 0; i < DAC_BUFFER_SIZE; i++) {		// Clear USB subsystem's buffer in order to mute I2S
							spk_buffer_0[i] = 0;
							spk_buffer_1[i] = 0;
						}

						#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)		// With WM8805 present, handle semaphores
							#ifdef USB_STATE_MACHINE_DEBUG
								print_dbg_char('g');					// Debug semaphore, lowercase letters for USB tasks
								if( xSemaphoreGive(input_select_semphr) == pdTRUE ) {
									input_select = MOBO_SRC_NONE;			// Indicate WM may take over control
									print_dbg_char(60); // '<'
								}
								else
									print_dbg_char(62); // '>'
							#else
								if( xSemaphoreGive(input_select_semphr) == pdTRUE )
									input_select = MOBO_SRC_NONE;			// Indicate WM may take over control
							#endif
//			    			mobo_led(FLED_DARK, FLED_YELLOW, FLED_DARK);	// Indicate silence detected by USB subsystem
						#endif
					}

/* BSB 20131031 New location of gap calculation code */

					// Calculate gap after N packets, NOT each time feedback endpoint is polled
					if (time_to_calculate_gap > 0)
						time_to_calculate_gap--;
					else {
						if (time_to_calculate_gap == -1)		// Immediately after a skip/insert and then again shortly
							time_to_calculate_gap = SPK1_PACKETS_PER_GAP_SKIP - 1;
						else									// Initially and a while after any skip/insert
							time_to_calculate_gap = SPK1_PACKETS_PER_GAP_CALCULATION - 1;
						if (usb_alternate_setting_out == 1) {	// Used with explicit feedback and not ADC data

							// Which buffer is in use, and does it truly correspond to the num_remaining value?
							// Read DAC_buf_DMA_read before and after num_remaining in order to determine validity
							DAC_buf_DMA_read_local = DAC_buf_DMA_read;
							num_remaining = spk_pdca_channel->tcr;

							// DAC_buf_DMA_read is valid
							if (DAC_buf_DMA_read_local == DAC_buf_DMA_read) {
								if (DAC_buf_USB_OUT != DAC_buf_DMA_read_local) { 	// CS4344 and USB using same buffer
									if ( spk_index < (DAC_BUFFER_SIZE - num_remaining))
										gap = DAC_BUFFER_SIZE - num_remaining - spk_index;
									else
										gap = DAC_BUFFER_SIZE - spk_index + DAC_BUFFER_SIZE - num_remaining + DAC_BUFFER_SIZE;
								}
								else // usb and pdca working on different buffers
									gap = (DAC_BUFFER_SIZE - spk_index) + (DAC_BUFFER_SIZE - num_remaining);
							}
							// DAC_buf_DMA_read is INVALID, don't calculate new gap.
							else {
								gap = old_gap;
								time_to_calculate_gap = -1;				// Immediate gap re-calculation
#ifdef USB_STATE_MACHINE_DEBUG
									print_dbg_char('R');
#endif
							}

							if(playerStarted) {

								if (FEATURE_NOSKIP_OFF) { 				// If skip/insert isn't disabled...
									if (gap < SPK1_GAP_LSKIP) {
										skip_enable |= SPK1_SKIP_EN_GAP;	// Enable skip/insert due to excessive buffer gap
									}
									else if (gap > SPK1_GAP_USKIP) {
										skip_enable |= SPK1_SKIP_EN_GAP;	// Enable skip/insert due to excessive buffer gap
									}
									else {
										skip_enable &= ~SPK1_SKIP_EN_GAP;	// Remove skip enable due to excessive buffer gap
									}
								}

								if (gap < old_gap) {
									if (gap < SPK1_GAP_L2) { 			// gap < outer lower bound => 2*FB_RATE_DELTA
										FB_rate -= 2*FB_RATE_DELTA;
										old_gap = gap;
										skip_indicate = 0;				// Feedback system is running again!
										LED_On(LED0);
#ifdef USB_STATE_MACHINE_DEBUG
										print_dbg_char('/');
#endif
									}
									else if (gap < SPK1_GAP_L1) { 		// gap < inner lower bound => 1*FB_RATE_DELTA
										FB_rate -= FB_RATE_DELTA;
										old_gap = gap;
										skip_indicate = 0;				// Feedback system is running again!
										LED_On(LED0);
#ifdef USB_STATE_MACHINE_DEBUG
										print_dbg_char('-');
#endif
									}
									else if (skip_indicate == 0) {		// Go back to indicating feedback system on module LEDs
										LED_Off(LED0);
										LED_Off(LED1);
									}
								}
								else if (gap > old_gap) {
									if (gap > SPK1_GAP_U2) { 			// gap > outer upper bound => 2*FB_RATE_DELTA
										FB_rate += 2*FB_RATE_DELTA;
										old_gap = gap;
										skip_indicate = 0;				// Feedback system is running again!
										LED_On(LED1);
#ifdef USB_STATE_MACHINE_DEBUG
										print_dbg_char('*');
#endif
									}
									else if (gap > SPK1_GAP_U1) { 		// gap > inner upper bound => 1*FB_RATE_DELTA
										FB_rate += FB_RATE_DELTA;
										old_gap = gap;
										skip_indicate = 0;				// Feedback system is running again!
										LED_On(LED1);
#ifdef USB_STATE_MACHINE_DEBUG
										print_dbg_char('+');
#endif
									}
									else if (skip_indicate == 0) {		// Go back to indicating feedback system on module LEDs
										LED_Off(LED0);
										LED_Off(LED1);
									}
								}
								else if (skip_indicate == 0) {		// Go back to indicating feedback system on module LEDs
									LED_Off(LED0);
									LED_Off(LED1);
								}
							} // end if(playerStarted)
						} // end if (usb_alternate_setting_out == 1)

					} // end if time_to_calculate_gap == 0

/* BSB 20131031 End of new location for gap calculation code */

				}	// end usb_out_received
			} // end usb_alternate_setting_out == 1


			else { // opposite of ( (usb_alternate_setting_out == 1) && (usb_ch_swap == USB_CH_NOSWAP) )
				playerStarted = FALSE;
				silence_USB = SILENCE_USB_LIMIT;				// Indicate USB silence

				#ifdef HW_GEN_DIN20								// Dedicated mute pin
					if (usb_ch_swap == USB_CH_SWAPDET)
						usb_ch_swap = USB_CH_SWAPACK;			// Acknowledge a USB channel swap, that takes this task into startup
				#endif

				if (input_select == MOBO_SRC_UAC1) {			// Set from playing nonzero USB
					#ifdef HW_GEN_DIN20							// Dedicated mute pin
						mobo_i2s_enable(MOBO_I2S_DISABLE);		// Hard-mute of I2S pin
					#endif
					// Silencing incoming (OUT endpoint) audio buffer for good measure. Resorting to this buffer is in fact muting the WM8805
					for (i = 0; i < DAC_BUFFER_SIZE; i++) {
						spk_buffer_0[i] = 0;
						spk_buffer_1[i] = 0;
					}

					#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)		// With WM8805 present, handle semaphores
						#ifdef USB_STATE_MACHINE_DEBUG
							print_dbg_char('h');				// Debug semaphore, lowercase letters for USB tasks
							if (xSemaphoreGive(input_select_semphr) == pdTRUE) {
								input_select = MOBO_SRC_NONE;
								print_dbg_char(60); // '<'
							}
							else
								print_dbg_char(62); // '>'
						#else
							if (xSemaphoreGive(input_select_semphr) == pdTRUE)
								input_select = MOBO_SRC_NONE;
						#endif
//	 			   		mobo_led(FLED_DARK, FLED_YELLOW, FLED_DARK);	// Indicate silence detected by USB subsystem
					#endif
				}
			}

			// BSB 20131201 attempting improved playerstarted detection
			if (usb_buffer_toggle == USB_BUFFER_TOGGLE_LIM)	{	// Counter is increased by DMA, decreased by seq. code
				usb_buffer_toggle = USB_BUFFER_TOGGLE_PARK;		// When it reaches limit, stop counting and park this mechanism
				playerStarted = FALSE;

#ifdef USB_STATE_MACHINE_DEBUG
				print_dbg_char('q');
#endif
			}

	} // end while vTask

}

#endif  // USB_DEVICE_FEATURE == ENABLED










