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

#if LCD_DISPLAY				// Multi-line LCD display
#include "taskLCD.h"
#endif

#include "composite_widget.h"
#include "taskAK5394A.h"

//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________


//_____ D E C L A R A T I O N S ____________________________________________

#define UAC2_NEW 1

#if UAC2_OLD
static U16  index, spk_index;
static U16  old_gap = SPK_BUFFER_SIZE;
static U8 audio_buffer_out, spk_buffer_in;	// the ID number of the buffer used for sending out
											// to the USB and reading from USB

static U8 ep_audio_in, ep_audio_out, ep_audio_out_fb;


//!
//! @brief Entry point of the device Audio task management
//!

stat void device_audio_task_old(void *pvParameters)
{
	static U32  time=0;
	static Bool startup=TRUE;
	int i;
	U16 num_samples, num_remaining, gap;

	U8 sample_MSB;
	U8 sample_SB;
	U8 sample_LSB;
	U8 sample_HSB;
	U32 sample;
	const U8 EP_AUDIO_IN = ep_audio_in;
	const U8 EP_AUDIO_OUT = ep_audio_out;
	const U8 EP_AUDIO_OUT_FB = ep_audio_out_fb;
	const U8 IN_LEFT = FEATURE_IN_NORMAL ? 0 : 1;
	const U8 IN_RIGHT = FEATURE_IN_NORMAL ? 1 : 0;
	const U8 OUT_LEFT = FEATURE_OUT_NORMAL ? 0 : 1;
	const U8 OUT_RIGHT = FEATURE_OUT_NORMAL ? 1 : 0;
	volatile avr32_pdca_channel_t *pdca_channel = pdca_get_handler(PDCA_CHANNEL_SSC_RX);
	volatile avr32_pdca_channel_t *spk_pdca_channel = pdca_get_handler(PDCA_CHANNEL_SSC_TX);

	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	while (TRUE) {
		vTaskDelayUntil(&xLastWakeTime, UAC2_configTSK_USB_DAUDIO_PERIOD);

		// First, check the device enumeration state
		if (!Is_device_enumerated()) { time=0; startup=TRUE; continue; };

		if( startup ) {


			time+=UAC2_configTSK_USB_DAUDIO_PERIOD;
#define STARTUP_LED_DELAY  10000
			if ( time<= 1*STARTUP_LED_DELAY ) {
				LED_On( LED0 );
				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_disable(PDCA_CHANNEL_SSC_RX);
			} else if( time== 2*STARTUP_LED_DELAY ) LED_On( LED1 );
			else if( time== 3*STARTUP_LED_DELAY ) LED_On( LED2 );
			else if( time== 4*STARTUP_LED_DELAY ) LED_On( LED3 );
			else if( time== 5*STARTUP_LED_DELAY ) LED_Off( LED0 );
			else if( time== 6*STARTUP_LED_DELAY ) LED_Off( LED1 );
			else if( time== 7*STARTUP_LED_DELAY ) LED_Off( LED2 );
			else if( time== 8*STARTUP_LED_DELAY ) LED_Off( LED3 );
			else if( time >= 9*STARTUP_LED_DELAY ) {
				startup=FALSE;

				audio_buffer_in = 0;
				audio_buffer_out = 0;
				spk_buffer_in = 0;
				spk_buffer_out = 0;
				index = 0;

				if (!FEATURE_ADC_NONE){
					// Wait for the next frame synchronization event
					// to avoid channel inversion.  Start with left channel - FS goes low
					while (!gpio_get_pin_value(AK5394_LRCK));
					while (gpio_get_pin_value(AK5394_LRCK));
					// Enable now the transfer.
					pdca_enable(PDCA_CHANNEL_SSC_RX);
					pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				}

				freq_changed = 1;						// force a freq change reset
			}
		}


		if (usb_alternate_setting == 1) {
			if (current_freq.frequency == 96000) num_samples = 24;
			else if (current_freq.frequency == 48000) num_samples = 12;
			else num_samples = 48;	// freq 192khz

			if (!FEATURE_ADC_NONE){
				if (Is_usb_in_ready(EP_AUDIO_IN)) {	// Endpoint ready for data transfer?

					Usb_ack_in_ready(EP_AUDIO_IN);	// acknowledge in ready

					// Sync AK data stream with USB data stream
					// AK data is being filled into ~audio_buffer_in, ie if audio_buffer_in is 0
					// buffer 0 is set in the reload register of the pdca
					// So the actual loading is occuring in buffer 1
					// USB data is being taken from audio_buffer_out

					// find out the current status of PDCA transfer
					// gap is how far the audio_buffer_out is from overlapping audio_buffer_in

					num_remaining = pdca_channel->tcr;
					if (audio_buffer_in != audio_buffer_out) {
						// AK and USB using same buffer
						if ( index < (AUDIO_BUFFER_SIZE - num_remaining)) gap = AUDIO_BUFFER_SIZE - num_remaining - index;
						else gap = AUDIO_BUFFER_SIZE - index + AUDIO_BUFFER_SIZE - num_remaining + AUDIO_BUFFER_SIZE;
					} else {
						// usb and pdca working on different buffers
						gap = (AUDIO_BUFFER_SIZE - index) + (AUDIO_BUFFER_SIZE - num_remaining);
					}

					if ( gap < AUDIO_BUFFER_SIZE/2 ) {
						// throttle back, transfer less
						num_samples--;
					} else if (gap > (AUDIO_BUFFER_SIZE + AUDIO_BUFFER_SIZE/2)) {
						// transfer more
						num_samples++;
					}

					Usb_reset_endpoint_fifo_access(EP_AUDIO_IN);
					for( i=0 ; i < num_samples ; i++ ) {
						   // Fill endpoint with samples
						if (!mute) {
							if (audio_buffer_out == 0) {
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

							if (audio_buffer_out == 0) {
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
							if (index >= AUDIO_BUFFER_SIZE) {
								index=0;
								audio_buffer_out = 1 - audio_buffer_out;
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
			} // end FEATURE_ADC
		} // end alt setting 1

		if (usb_alternate_setting_out == 1){

			if (Is_usb_in_ready(EP_AUDIO_OUT_FB)) {	// Endpoint buffer free ?
				Usb_ack_in_ready(EP_AUDIO_OUT_FB);	// acknowledge in ready

				Usb_reset_endpoint_fifo_access(EP_AUDIO_OUT_FB);
				// Sync CS4344 spk data stream by calculating gap and provide feedback
				num_remaining = spk_pdca_channel->tcr;
				if (spk_buffer_in != spk_buffer_out) {
					// CS4344 and USB using same buffer
					
					if ( spk_index < (SPK_BUFFER_SIZE - num_remaining)) gap = SPK_BUFFER_SIZE - num_remaining - spk_index;
					else gap = SPK_BUFFER_SIZE - spk_index + SPK_BUFFER_SIZE - num_remaining + SPK_BUFFER_SIZE;
				} else {
					// usb and pdca working on different buffers
					gap = (SPK_BUFFER_SIZE - spk_index) + (SPK_BUFFER_SIZE - num_remaining);
				}

				if (Is_usb_full_speed_mode()) {			// FB rate is 3 bytes in 10.14 format

					if ((gap < (SPK_BUFFER_SIZE/2)) && (gap < old_gap)) {
						LED_Toggle(LED0);
						FB_rate -= 32;
						old_gap = gap;
					} else if ( (gap > (SPK_BUFFER_SIZE + (SPK_BUFFER_SIZE/2))) && (gap > old_gap)) {
						LED_Toggle(LED1);
						FB_rate += 32;
						old_gap = gap;
					}

					sample_LSB = FB_rate;
					sample_SB = FB_rate >> 8;
					sample_MSB = FB_rate >> 16;
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_LSB);
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_SB);
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_MSB);
				} else {
					// HS mode
					// FB rate is 4 bytes in 12.13 format

					if ((gap < (SPK_BUFFER_SIZE/2)) && (gap < old_gap)){
						LED_Toggle(LED0);
						FB_rate -= 16;
						old_gap = gap;
					} else if ( (gap > (SPK_BUFFER_SIZE + (SPK_BUFFER_SIZE/2))) && (gap > old_gap)) {
						LED_Toggle(LED1);
						FB_rate += 16;
						old_gap = gap;
					}

					sample_LSB = FB_rate;
					sample_SB = FB_rate >> 8;
					sample_MSB = FB_rate >> 16;
					sample_HSB = FB_rate >> 24;
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_LSB);
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_SB);
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_MSB);
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_HSB);
				}

				Usb_send_in(EP_AUDIO_OUT_FB);
			} // end sub_in_ready


			if (Is_usb_out_received(EP_AUDIO_OUT)) {

				Usb_reset_endpoint_fifo_access(EP_AUDIO_OUT);
				num_samples = Usb_byte_count(EP_AUDIO_OUT) / 6;

				for (i = 0; i < num_samples; i++) {
					if (spk_mute) {
						sample_LSB = 0;
						sample_SB = 0;
						sample_MSB = 0;
					} else {
						sample_LSB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						sample_SB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						sample_MSB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
					};

					sample = (((U32) sample_MSB) << 16) + (((U32)sample_SB) << 8) + sample_LSB;
					if (spk_buffer_in == 0) spk_buffer_0[spk_index+OUT_LEFT] = sample;
					else spk_buffer_1[spk_index+OUT_LEFT] = sample;

					if (spk_mute) {
						sample_LSB = 0;
						sample_SB = 0;
						sample_MSB = 0;
					} else {
						sample_LSB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						sample_SB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						sample_MSB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
					};

					sample = (((U32) sample_MSB) << 16) + (((U32)sample_SB) << 8) + sample_LSB;
					if (spk_buffer_in == 0) spk_buffer_0[spk_index+OUT_RIGHT] = sample;
					else spk_buffer_1[spk_index+OUT_RIGHT] = sample;

					spk_index += 2;
					if (spk_index >= SPK_BUFFER_SIZE){
						spk_index = 0;
						spk_buffer_in = 1 - spk_buffer_in;
					}
				}
				Usb_ack_out_received_free(EP_AUDIO_OUT);
			}	// end usb_out_received
		} // end usb_alternate_setting_out == 1
		
	} // end while vTask

}
#endif

#if UAC2_NEW

static struct {
	U8 ep_in, ep_out, ep_out_fb;
	U16 daudio_period;
} params;
 
//!
//! @brief Entry point of the device Audio task management
//!

static void device_audio_task(void *pvParameters)
{
	U32  time=0;
	Bool startup=TRUE;
	int i;
	U16 num_samples, num_remaining, gap;

	const U8 EP_AUDIO_IN = params.ep_in;
	const U8 EP_AUDIO_OUT = params.ep_out;
	const U8 EP_AUDIO_OUT_FB = params.ep_out_fb;
	const U16 USB_DAUDIO_PERIOD = params.daudio_period;
	const U8 IN_LEFT = FEATURE_IN_NORMAL ? 0 : 1;
	const U8 IN_RIGHT = FEATURE_IN_NORMAL ? 1 : 0;
	const U8 OUT_LEFT = FEATURE_OUT_NORMAL ? 0 : 1;
	const U8 OUT_RIGHT = FEATURE_OUT_NORMAL ? 1 : 0;
	const U8 DO_INPUT = EP_AUDIO_IN != 0 && ! FEATURE_ADC_NONE;
	const U8 DO_OUTPUT = EP_AUDIO_OUT != 0 && ! FEATURE_DAC_NONE;
	const U8 DO_OUTPUT_FEEDBACK = EP_AUDIO_OUT_FB != 0 && DO_OUTPUT;

	volatile avr32_pdca_channel_t *pdca_channel = pdca_get_handler(PDCA_CHANNEL_SSC_RX);
	volatile avr32_pdca_channel_t *spk_pdca_channel = pdca_get_handler(PDCA_CHANNEL_SSC_TX);

	U8 audio_buffer_out = 0, spk_buffer_in = 0;
	U16 index = 0, spk_index = 0;
	S16 count = SPK_BUFFER_SIZE, spk_count = SPK_BUFFER_SIZE;
	volatile U32 *ptr = audio_buffer[audio_buffer_out], *spk_ptr = spk_buffer[spk_buffer_in];
	U16 old_gap = SPK_BUFFER_SIZE;

	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	while (TRUE) {
		vTaskDelayUntil(&xLastWakeTime, USB_DAUDIO_PERIOD);

		// First, check the device enumeration state
		if ( ! Is_device_enumerated() ) {
			time=0;
			startup=TRUE;
			continue;
		}

		if( startup ) {

			time += USB_DAUDIO_PERIOD;
#define STARTUP_LED_DELAY  4000
			if ( time<= 1*STARTUP_LED_DELAY ) {
				LED_On( LED0 );
				pdca_disable(PDCA_CHANNEL_SSC_RX);
				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
			} else if( time== 2*STARTUP_LED_DELAY ) LED_On( LED1 );
			else if( time== 3*STARTUP_LED_DELAY ) LED_On( LED2 );
			else if( time== 4*STARTUP_LED_DELAY ) LED_On( LED3 );
			else if( time== 5*STARTUP_LED_DELAY ) {
				LED_Off( LED0 );
				//            gpio_set_gpio_pin(AK5394_RSTN);		// start AK5394A
			} else if( time== 6*STARTUP_LED_DELAY ) LED_Off( LED1 );
			else if( time== 7*STARTUP_LED_DELAY ) LED_Off( LED2 );
			else if( time== 8*STARTUP_LED_DELAY ) LED_Off( LED3 );
			else if( time >= 9*STARTUP_LED_DELAY ) {
				startup=FALSE;

				audio_buffer_in = 0;
				audio_buffer_out = 0;
				index = 0;
				count = SPK_BUFFER_SIZE;
				ptr = audio_buffer[audio_buffer_out];
				spk_buffer_in = 0;
				spk_buffer_out = 0;
				spk_index = 0;
				spk_count = SPK_BUFFER_SIZE;
				spk_ptr = spk_buffer[spk_buffer_in];

				if ( ! FEATURE_ADC_NONE ) {
					// Wait for the next frame synchronization event
					// to avoid channel inversion.  Start with left channel - FS goes low
					while (!gpio_get_pin_value(AK5394_LRCK));
					while (gpio_get_pin_value(AK5394_LRCK));
					// Enable now the transfer.
					pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
					pdca_enable(PDCA_CHANNEL_SSC_RX);
				}

				freq_changed = 1;						// force a freq change reset
			}
		}


		if (DO_INPUT && usb_alternate_setting == 1) {
			if (current_freq.frequency == 96000) num_samples = 24;
			else if (current_freq.frequency == 48000) num_samples = 12;
			else num_samples = 48;	// freq 192khz

			if (Is_usb_in_ready(EP_AUDIO_IN)) {	// Endpoint ready for data transfer?

				Usb_ack_in_ready(EP_AUDIO_IN);	// acknowledge in ready

				// Sync AK data stream with USB data stream
				// AK data is being filled into ~audio_buffer_in, ie if audio_buffer_in is 0
				// buffer 0 is set in the reload register of the pdca
				// So the actual loading is occuring in buffer 1
				// USB data is being taken from audio_buffer_out

				// find out the current status of PDCA transfer
				// gap is how far the audio_buffer_out is from overlapping audio_buffer_in

				num_remaining = pdca_channel->tcr;
				if (audio_buffer_in != audio_buffer_out) {
					// AK and USB using same buffer
					if ( index < (AUDIO_BUFFER_SIZE - num_remaining)) gap = AUDIO_BUFFER_SIZE - num_remaining - index;
					else gap = AUDIO_BUFFER_SIZE - index + AUDIO_BUFFER_SIZE - num_remaining + AUDIO_BUFFER_SIZE;
				} else {
					// usb and pdca working on different buffers
					gap = (AUDIO_BUFFER_SIZE - index) + (AUDIO_BUFFER_SIZE - num_remaining);
				}

				if ( gap < AUDIO_BUFFER_SIZE/2 ) {
					// throttle back, transfer less
					num_samples--;
				} else if (gap > (AUDIO_BUFFER_SIZE + AUDIO_BUFFER_SIZE/2)) {
					// transfer more
					num_samples++;
				}

				Usb_reset_endpoint_fifo_access(EP_AUDIO_IN);

				const U32 MUTE_MASK = mute ? 0x00000000 : 0xFFFFFFFF;

				for( i=0 ; i < num_samples ; i++ ) {
					// Fill endpoint with samples

					U32 sample = ptr[IN_LEFT] & MUTE_MASK;

					Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample);
					Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample>>8);
					Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample>>16);

					sample = ptr[IN_RIGHT] & MUTE_MASK;

					Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample);
					Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample>>8);
					Usb_write_endpoint_data(EP_AUDIO_IN, 8, sample>>16);
					
					ptr += 2;
					index += 2;
					count -= 2;
					if (count <= 0) {
						audio_buffer_out ^= 1;
						index=0;
						count = SPK_BUFFER_SIZE;
						ptr = audio_buffer[audio_buffer_out];
					}
				}
				Usb_send_in(EP_AUDIO_IN);		// send the current bank
			} // end Is_usb_in_ready(EP_AUDIO_IN)
		} // end DO_INPUT && usb_alternate_setting == 1
		
		if (DO_OUTPUT && usb_alternate_setting_out == 1){

			if (DO_OUTPUT_FEEDBACK && Is_usb_in_ready(EP_AUDIO_OUT_FB)) {	// Endpoint buffer free ?
				Usb_ack_in_ready(EP_AUDIO_OUT_FB);	// acknowledge in ready

				Usb_reset_endpoint_fifo_access(EP_AUDIO_OUT_FB);
				// Sync CS4344 spk data stream by calculating gap and provide feedback
				num_remaining = spk_pdca_channel->tcr;
				if (spk_buffer_in != spk_buffer_out) {
					// CS4344 and USB using same buffer
					
					if ( spk_index < (SPK_BUFFER_SIZE - num_remaining)) gap = SPK_BUFFER_SIZE - num_remaining - spk_index;
					else gap = SPK_BUFFER_SIZE - spk_index + SPK_BUFFER_SIZE - num_remaining + SPK_BUFFER_SIZE;
				} else {
					// usb and pdca working on different buffers
					gap = (SPK_BUFFER_SIZE - spk_index) + (SPK_BUFFER_SIZE - num_remaining);
				}

				if (Is_usb_full_speed_mode()) {			// FB rate is 3 bytes in 10.14 format

					if ((gap < (SPK_BUFFER_SIZE/2)) && (gap < old_gap)) {
						LED_Toggle(LED0);
						FB_rate -= 32;
						old_gap = gap;
					} else if ( (gap > (SPK_BUFFER_SIZE + (SPK_BUFFER_SIZE/2))) && (gap > old_gap)) {
						LED_Toggle(LED1);
						FB_rate += 32;
						old_gap = gap;
					}

					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, FB_rate);
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, FB_rate >> 8);
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, FB_rate >> 16);
				} else {
					// HS mode
					// FB rate is 4 bytes in 12.13 format

					if ((gap < (SPK_BUFFER_SIZE/2)) && (gap < old_gap)){
						LED_Toggle(LED0);
						FB_rate -= 16;
						old_gap = gap;
					} else if ( (gap > (SPK_BUFFER_SIZE + (SPK_BUFFER_SIZE/2))) && (gap > old_gap)) {
						LED_Toggle(LED1);
						FB_rate += 16;
						old_gap = gap;
					}

					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, FB_rate);
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, FB_rate >> 8);
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, FB_rate >> 16);
					Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, FB_rate >> 24);
				}

				Usb_send_in(EP_AUDIO_OUT_FB);
			} // end DO_OUTPUT_FEEDBACK && Is_usb_in_ready(EP_AUDIO_OUT_FB)


			if (Is_usb_out_received(EP_AUDIO_OUT)) {

				Usb_reset_endpoint_fifo_access(EP_AUDIO_OUT);
				num_samples = Usb_byte_count(EP_AUDIO_OUT) / 6;

				const U32 SPK_MUTE_MASK = spk_mute ? 0x00000000 : 0xFFFFFFFF;
				for (i = 0; i < num_samples; i++) {
					U32 sample = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
					sample |= ((U32)Usb_read_endpoint_data(EP_AUDIO_OUT, 8)) << 8;
					sample |= ((U32)Usb_read_endpoint_data(EP_AUDIO_OUT, 8)) << 16;

					spk_ptr[OUT_LEFT] = sample & SPK_MUTE_MASK;

					sample = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
					sample |= ((U32)Usb_read_endpoint_data(EP_AUDIO_OUT, 8)) << 8;
					sample |= ((U32)Usb_read_endpoint_data(EP_AUDIO_OUT, 8)) << 16;

					spk_ptr[OUT_RIGHT] = sample & SPK_MUTE_MASK;

					spk_ptr += 2;
					spk_index += 2;
					spk_count -= 2;
					if (spk_count <= 0){
						spk_index = 0;
						spk_count = SPK_BUFFER_SIZE;
						spk_buffer_in ^= 1;
						spk_ptr = spk_buffer[spk_buffer_in];
					}
				}
				Usb_ack_out_received_free(EP_AUDIO_OUT);
			}	// end usb_out_received
		} // end usb_alternate_setting_out == 1
	} // end while vTask
}
#endif
//!
//! @brief This function initializes the hardware/software resources
//! required for device Audio task.
//!
void uac2_device_audio_task_init(U8 ep_in, U8 ep_out, U8 ep_out_fb)
{
#if UAC2_OLD
	index     =0;
	audio_buffer_out = 0;
	spk_index = 0;
	spk_buffer_in = 0;
#endif
	mute = FALSE;
	spk_mute = FALSE;
#if UAC2_OLD	
	ep_audio_in = ep_in;
	ep_audio_out = ep_out;
	ep_audio_out_fb = ep_out_fb;
#endif
#if UAC2_NEW
	params.ep_in = ep_in;
	params.ep_out = ep_out;
	params.ep_out_fb = ep_out_fb;
	params.daudio_period = UAC2_configTSK_USB_DAUDIO_PERIOD;
#endif

	xTaskCreate(device_audio_task,
				configTSK_USB_DAUDIO_NAME,
				configTSK_USB_DAUDIO_STACK_SIZE,
				NULL,
				configTSK_USB_DAUDIO_PRIORITY,
				NULL);
}

#endif  // USB_DEVICE_FEATURE == ENABLED
