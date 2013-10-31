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

//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________


// #define FB_RATE_DELTA (1<<12)
#define FB_RATE_DELTA 64 // BSB 20130603 stability??
#define FB_RATE_DELTA_NUM 2

// BSB 20130605 FB_rate calculation with 2 levels, imported from UAC2 code
#define SPK1_GAP_U2	SPK_BUFFER_SIZE * 6 / 4	// 6 A half buffer up in distance	=> Speed up host a lot
#define	SPK1_GAP_U1	SPK_BUFFER_SIZE * 5 / 4	// 5 A quarter buffer up in distance => Speed up host a bit
#define SPK1_GAP_NOM	SPK_BUFFER_SIZE	* 4 / 4	// 4 Ideal distance is half the size of linear buffer
#define SPK1_GAP_L1	SPK_BUFFER_SIZE * 3 / 4 // 3 A quarter buffer down in distance => Slow down host a bit
#define SPK1_GAP_L2	SPK_BUFFER_SIZE * 2 / 4 // 2 A half buffer down in distance => Slow down host a lot
#define	SPK1_PACKETS_PER_GAP_CALCULATION 32		// This is UAC1 which counts in ms. Gap calculation every 32ms


//_____ D E C L A R A T I O N S ____________________________________________


//? why are these defined as statics?

static U32  index, spk_index;
static U16  old_gap = SPK_BUFFER_SIZE;
static U8 audio_buffer_out, spk_buffer_in;	// the ID number of the buffer used for sending out to the USB
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
	audio_buffer_out = 0;
	audio_buffer_ptr = audio_buffer_0;
	spk_index = 0;
	spk_buffer_in = 0;
//	spk_buffer_ptr = spk_buffer_0;
	mute = FALSE;
	spk_mute = FALSE;
	volume = 0x5000;
	spk_volume = 0x5000;
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
	Bool playerStarted = FALSE;
	static U32  time=0;
	static Bool startup=TRUE;
	int i;
//	int delta_num = 0;
	U16 num_samples, num_remaining, gap, time_to_calculate_gap;
	U8 sample_HSB;
	U8 sample_MSB;
	U8 sample_SB;
	U8 sample_LSB;
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
	// BSB 20130602: code section moved to uac1_usb_specific_request.c
	// if (current_freq.frequency == 48000) FB_rate = 48 << 14;
	// else FB_rate = (44 << 14) + (1 << 14)/10;

/*  BSB debug 20130602
 *  attempt to write one in 100-ish feedback packets to UART by spacing out ASCII'ified HEX characters
 *  Unfinished code
 	U8 s_counter = 0;
	U8 s_HEX[10];

void uart_puthex(uint8_t c) {
    int8_t temp = c;

    c >>=4;                                    			// Shift in most significant hex character
    if (c < 10)                                			// 0-9 -> '0' - '9' (48 decimal is ascii for '0')
        uart_putc(c + 48);
    else                                        		// A-F -> 'A' - 'F' (65 decimal is ascii for 'A')
        uart_putc(c - 0x0A + 65);

    c = temp & 0x0F;                            		// Mask in least significant hex character
    if (c < 10)                                  		// 0-9 -> '0' - '9'
        uart_putc(c + 48);
	else                                        		// A-F -> 'A' - 'F'
        uart_putc(c - 0x0A + 65);
}
 */

	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	while (TRUE) {
		vTaskDelayUntil(&xLastWakeTime, UAC1_configTSK_USB_DAUDIO_PERIOD);

		// First, check the device enumeration state
		if (!Is_device_enumerated()) { time=0; startup=TRUE; continue; };

		if( startup ) {


			time+=UAC1_configTSK_USB_DAUDIO_PERIOD;
#define STARTUP_LED_DELAY  10000
			if ( time<= 1*STARTUP_LED_DELAY ) {
				LED_On( LED0 );
				pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_disable(PDCA_CHANNEL_SSC_RX);
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
			}
		}
		//else {

			num_samples = 48;

			if (usb_alternate_setting == 1) {

				if (Is_usb_in_ready(EP_AUDIO_IN)) {	// Endpoint buffer free ?
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


					// Sync the USB stream with the AK stream
					// throttle back
					if  (gap < AUDIO_BUFFER_SIZE/2) {
						num_samples--;
					} else {
						// speed up
						if  (gap > (AUDIO_BUFFER_SIZE + AUDIO_BUFFER_SIZE/2)) {
							num_samples++;
						}
					}

					Usb_reset_endpoint_fifo_access(EP_AUDIO_IN);
					for( i=0 ; i < num_samples ; i++ ) {
						// Fill endpoint with sample raw
						if (mute==FALSE) {
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
			} // end alt setting == 1

			if (usb_alternate_setting_out == 1) {
				if ( Is_usb_in_ready(EP_AUDIO_OUT_FB) )
				{
					Usb_ack_in_ready(EP_AUDIO_OUT_FB);	// acknowledge in ready
					Usb_reset_endpoint_fifo_access(EP_AUDIO_OUT_FB);

#ifdef USB_STATE_MACHINE_DEBUG
					gpio_clr_gpio_pin(AVR32_PIN_PX31); // BSB 20130602 debug on GPIO_07
#endif

					if (Is_usb_full_speed_mode()) {			// FB rate is 3 bytes in 10.14 format
						sample_LSB = FB_rate;
						sample_SB = FB_rate >> 8;
						sample_MSB = FB_rate >> 16;
						Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_LSB);
						Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_SB);
						Usb_write_endpoint_data(EP_AUDIO_OUT_FB, 8, sample_MSB);
					}
					else {
						// HS mode
						// FB rate is 4 bytes in 12.14 format
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

#ifdef USB_STATE_MACHINE_DEBUG
					gpio_set_gpio_pin(AVR32_PIN_PX31); // BSB 20130602 debug on GPIO_07
#endif
				}

				if (Is_usb_out_received(EP_AUDIO_OUT)) {
					spk_usb_heart_beat++;			// indicates EP_AUDIO_OUT receiving data from host

					Usb_reset_endpoint_fifo_access(EP_AUDIO_OUT);
					num_samples = Usb_byte_count(EP_AUDIO_OUT) / 6;

					if(!playerStarted) {
						time_to_calculate_gap = 0;			// BSB 20131031 moved gap calculation for DAC use
						playerStarted = TRUE;
						num_remaining = spk_pdca_channel->tcr;
						spk_buffer_in = spk_buffer_out;

#ifdef USB_STATE_MACHINE_DEBUG
						if (spk_buffer_in == 1)
							gpio_set_gpio_pin(AVR32_PIN_PX55); // BSB 20120911 debug on GPIO_03
						else
							gpio_clr_gpio_pin(AVR32_PIN_PX55); // BSB 20120911 debug on GPIO_03
#endif

						spk_index = SPK_BUFFER_SIZE - num_remaining;

						spk_index = spk_index & ~((U32)1); // Clear LSB in order to start with L sample
					}

					for (i = 0; i < num_samples; i++) {
						if (spk_mute) {
							sample_LSB = 0;
							sample_SB = 0;
							sample_MSB = 0;
						} else {
							sample_LSB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
							sample_SB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
							sample_MSB = Usb_read_endpoint_data(EP_AUDIO_OUT, 8);
						}

						sample = (((U32) sample_MSB) << 16) + (((U32)sample_SB) << 8) + sample_LSB;
						if (spk_buffer_in == 0)
							spk_buffer_0[spk_index+OUT_LEFT] = sample;
						else
							spk_buffer_1[spk_index+OUT_LEFT] = sample;

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
						if (spk_buffer_in == 0)
							spk_buffer_0[spk_index+OUT_RIGHT] = sample;
						else
							spk_buffer_1[spk_index+OUT_RIGHT] = sample;

						spk_index += 2;
						if (spk_index >= SPK_BUFFER_SIZE) {
							spk_index = 0;
							spk_buffer_in = 1 - spk_buffer_in;

#ifdef USB_STATE_MACHINE_DEBUG
							if (spk_buffer_in == 1)
								gpio_set_gpio_pin(AVR32_PIN_PX55); // BSB 20120912 debug on GPIO_03
							else
								gpio_clr_gpio_pin(AVR32_PIN_PX55); // BSB 20120912 debug on GPIO_03
#endif

						}
					}
					Usb_ack_out_received_free(EP_AUDIO_OUT);


/* BSB 20131031 New location of gap calculation code */

					if (time_to_calculate_gap != 0)
						time_to_calculate_gap--;
					else {
						time_to_calculate_gap = SPK1_PACKETS_PER_GAP_CALCULATION;

						if (usb_alternate_setting_out == 1) {
							// Sync CS4344 spk data stream by calculating gap and provide feedback
							num_remaining = spk_pdca_channel->tcr;
							if (spk_buffer_in != spk_buffer_out) {
								// CS4344 and USB using same buffer
								if ( spk_index < (SPK_BUFFER_SIZE - num_remaining))
									gap = SPK_BUFFER_SIZE - num_remaining - spk_index;
								else
									gap = SPK_BUFFER_SIZE - spk_index + SPK_BUFFER_SIZE - num_remaining + SPK_BUFFER_SIZE;
							}
							else {
								// usb and pdca working on different buffers
								gap = (SPK_BUFFER_SIZE - spk_index) + (SPK_BUFFER_SIZE - num_remaining);
							}

							if(playerStarted) {
								if (gap < old_gap) {
									if (gap < SPK1_GAP_L2) { 		// gap < outer lower bound => 2*FB_RATE_DELTA
										LED_On(LED0);
										FB_rate -= 2*FB_RATE_DELTA;
										old_gap = gap;
#ifdef USB_STATE_MACHINE_DEBUG
										print_dbg_char_char('/');
#endif
									}
									else if (gap < SPK1_GAP_L1) { 	// gap < inner lower bound => 1*FB_RATE_DELTA
										LED_On(LED0);
										FB_rate -= FB_RATE_DELTA;
										old_gap = gap;
#ifdef USB_STATE_MACHINE_DEBUG
										print_dbg_char_char('-');
#endif
									}
									else {
										LED_Off(LED0);
										LED_Off(LED1);
									}
								}
								else if (gap > old_gap) {
									if (gap > SPK1_GAP_U2) { 		// gap > outer upper bound => 2*FB_RATE_DELTA
										LED_On(LED1);
										FB_rate += 2*FB_RATE_DELTA;
										old_gap = gap;
#ifdef USB_STATE_MACHINE_DEBUG
										print_dbg_char_char('*');
#endif
									}
									else if (gap > SPK1_GAP_U1) { 	// gap > inner upper bound => 1*FB_RATE_DELTA
										LED_On(LED1);
										FB_rate += FB_RATE_DELTA;
										old_gap = gap;
#ifdef USB_STATE_MACHINE_DEBUG
										print_dbg_char_char('+');
#endif
									}
									else {
										LED_Off(LED0);
										LED_Off(LED1);
									}
								}
								else {
									LED_Off(LED0);
									LED_Off(LED1);
								}
							} // end if(playerStarted)

						} // end if (usb_alternate_setting_out == 1)

					} // end if time_to_calculate_gap == 0

/* BSB 20131031 End of new location for gap calculation code */


				}	// end usb_out_received
			} // end usb_alternate_setting_out == 1
			else
				playerStarted = FALSE;
		//}	// end startup else
	} // end while vTask

}

#endif  // USB_DEVICE_FEATURE == ENABLED
