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

This is valid for WM8805 code base has documentation that is not relevant for 
this WM8804 implementation. Must write new intro!

If this project is of interest to you, please let me know! I hope to see you at either:
 Audio Widget mailing list: audio-widget@googlegroups.com
 https://www.facebook.com/henryaudio
 borge@henryaudio.com

*/


#if (defined HW_GEN_SPRX)		// Functions here only make sense for WM8804

#include "wm8804.h"
#include "gpio.h"
// #include "Mobo_config.h"
#include "features.h"
#include "device_audio_task.h"
#include "usb_specific_request.h"
#include "Mobo_config.h"
#include "I2C.h"
#include "pdca.h" // To disable DMA at sleep
#include "taskAK5394A.h" // To signal uacX_device_audio_task to enable DMA at init

// Global status variable
volatile spdif_rx_status_t spdif_rx_status = {0, SILENCE_SPDIF_LIMIT, FREQ_TIMEOUT, WM8804_PLL_NONE, MOBO_SRC_NONE, MOBO_SRC_SPDIF0};
	
// Linkup monitoring variables, available for debug
volatile uint8_t link_attempts_max = 0;				// Counting up to determine max poll cycles for linkup success
volatile uint8_t link_attempts_min = 0xFF;			// Counting down to determine min poll cycles for linkup success

#ifdef FEATURE_SPDIF_CMD
	volatile uint8_t spdif_enable_state_machine = TRUE; // Run with SM to begin with 
#endif

// Using the WM8804 requires intimate knowledge of the chip and its datasheet. For this
// reason we use a lot of raw hex rather than naming of its internal registers.

// Hardware reset over GPIO pin. Consider the Power Up Configuration section of the datasheet!
void wm8804_reset(uint8_t reset_type) {
	if (reset_type == WM8804_RESET_START) {
		// 20170423 new
		gpio_set_gpio_pin(AVR32_PIN_PA15);			// SDA must be 1 at reset for SW. NB This will conflict with I2C!
		gpio_clr_gpio_pin(WM8804_SWIFMODE_PIN);		// HW_GEN_SPRX new feature under software control, hard pull down, 1k on PCB, AKA WM8804_ZERO_PIN

		gpio_clr_gpio_pin(WM8804_RESET_PIN);		// Clear reset pin WM8804 active low reset
		gpio_clr_gpio_pin(WM8804_CSB_PIN);			// CSB/GPO2 pin sets 2W address. Make sure CSB outputs 0.
	}
	else {
		gpio_set_gpio_pin(WM8804_RESET_PIN);		// Set reset pin WM8804 active low reset
		gpio_enable_gpio_pin(WM8804_CSB_PIN);		// CSB/GPO2 should now be an MCU input...
		gpio_enable_gpio_pin(WM8804_SWIFMODE_PIN);	// HW_GEN_SPRX new feature under software control, should now be an MCU input

		// 20170423 new
		gpio_enable_gpio_pin(AVR32_PIN_PA15);		// SDA should now be an MCU input... Or functional I2C or something....
	} 
}


// Start up the WM8804 config task 
void wm8804_task_init(void) {
	
	// Call all init code - wm8804_task_init now called where this used to be called
	
	#ifdef FEATURE_SPDIF_CMD
		if (spdif_enable_state_machine) {
	#else
		if (1) {
	#endif
			wm8804_init();									// Start up the WM8804 in a fairly dead mode
		} // ifdef FEATURE_SPDIF_CMD .. if()
	
	#ifdef FREERTOS_USED
		xTaskCreate(wm8804_task,
			configTSK_WM8804_NAME,
			configTSK_WM8804_STACK_SIZE,
			NULL,
			configTSK_WM8804_PRIORITY,
			NULL);
	#else
		#error No FreeRTOS. This will not work for you!
	#endif  // FREERTOS_USED
}


// Enable MCLK output on the CLKOUT (9) pin
uint8_t wm8804_mclk_out(uint8_t mode) {
	static uint8_t mode_recall = WM8804_MCLK_DISABLE;	// default setting is to disable MCLK output and run on XOs
	
	if ( (mode == WM8804_MCLK_ENABLE) || (mode == WM8804_MCLK_DISABLE) ) {
		uint8_t temp = wm8804_read_byte(0x08);

		if (mode == WM8804_MCLK_ENABLE) {
			temp = temp | 0b00010000;					// Turn bit 4 on
		}
		else { // implicitly: disable
			temp = temp & 0b11101111;					// Turn bit 4 off
		}

		wm8804_write_byte(0x08, temp);
		mode_recall = mode;
	}
	
	// else if mode == WM8804_MCLK_RECALL is redundant
	
	return mode_recall;
}

 
// The config task itself
void wm8804_task(void *pvParameters) {
	static uint8_t scanmode = WM8804_SCAN_FROM_NEXT + 0x05;		// Start scanning from next channel. Run up to 5x4 scan attempts
//	static uint8_t scanmode = WM8804_SCAN_FROM_PRESENT + 0x05;		// Start scanning from the last known good (?) channel. Run up to 5x4 scan attempts
	uint32_t freq;
	static uint8_t channel;	// Must be static here?
	uint8_t wm8804_int;
	uint8_t mustgive = 0;
	int16_t poll_counter = 0;

	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();			// Currently happens every 20ms with configTSK_WM8804_PERIOD = 200, every 1.2ms with configTSK_WM8804_PERIOD = 12

	while (TRUE) {
		
//		gpio_tgl_gpio_pin(AVR32_PIN_PX31);			// Indicate execution slots of this task
		
		vTaskDelayUntil(&xLastWakeTime, configTSK_WM8804_PERIOD);
		
		
		// Start of command handler
		#ifdef FEATURE_SPDIF_CMD
			uint8_t temp_spdif_u8 = 0;
			uint8_t temp_spdif_u32 = 0;

			switch (spdif_cmd) {
				case SPDIF_CMD_TAKE:
					// Semaphore input_select take
				break;
				case SPDIF_CMD_GIVE:
					// Semaphore input_select give
				break;
				case SPDIF_CMD_NONE:
				case SPDIF_CMD_UAC2:
				case SPDIF_CMD_SPDIF0:
				case SPDIF_CMD_TOSLINK1:
				case SPDIF_CMD_TOSLINK0:
				case SPDIF_CMD_SPDIF1:
					mobo_SPRX_input(spdif_cmd - 0x10);
					if (input_select == MOBO_SRC_NONE) {
						input_select = spdif_cmd - 0x10;
					}
					print_dbg_char_char('.');
					print_dbg_char_hex(input_select);
				break;
				case SPDIF_CMD_LINKSTATS:
					wm8804_linkstats();
				break;
				case SPDIF_CMD_SLEEP:
					wm8804_sleep();
				break;
				case SPDIF_CMD_WAKE:
				case SPDIF_CMD_INIT:
					wm8804_init();
				break;
				case SPDIF_CMD_SM_ON:
					spdif_enable_state_machine = TRUE;				
				break;
				case SPDIF_CMD_SM_OFF:
					spdif_enable_state_machine = FALSE;
				break;
				case SPDIF_CMD_MSRD:
					print_dbg_char_char('.');
					print_dbg_char_hex(mobo_srd() >> 10);	// Output is hex ~ksps
				break;
				case SPDIF_CMD_RSRD:
					temp_spdif_u8 = wm8804_read_byte(0x0C);
					print_dbg_char_char('.');
					print_dbg_char_hex(temp_spdif_u8 & 0b00110000); // 00 192/176, 10 96/88, 20 48/44, 30 32
				break;
				case SPDIF_CMD_I2SDIS:		// Not active at the moment
					mobo_i2s_enable(MOBO_I2S_DISABLE);
				break;
				case SPDIF_CMD_I2SEN:		// Not active at the moment
					mobo_i2s_enable(MOBO_I2S_ENABLE);
				break;
				case SPDIF_CMD_LIVEDET:
					print_dbg_char_hex(wm8804_live_detect());
				break;
				case SPDIF_CMD_INTS:
					temp_spdif_u8 = wm8804_read_byte(0x0B);
					print_dbg_char_char('.');
					print_dbg_char_hex(temp_spdif_u8);
				break;
				case SPDIF_CMD_MUTE:
					wm8804_mute();
				break;
				case SPDIF_CMD_UNMUTE:
					wm8804_unmute();
				break;
				case SPDIF_CMD_CLK_START:
					temp_spdif_u32 = mobo_srd();
					spdif_rx_status.frequency = temp_spdif_u32;
					wm8804_clkdivnew(temp_spdif_u32);
					// We should (have) set input_select by now or around here
					must_init_xo = TRUE;
				break;
				case SPDIF_WM_PLL_ALL:		// General purpose PLL, function only overwrites if needed
					wm8804_pllnew(WM8804_PLL_NORMAL);
				break;
				case SPDIF_WM_PLL_192:		// 192ksps PLL, function only overwrites if needed
					wm8804_pllnew(WM8804_PLL_192);
				break;
				case SPDIF_WM_PLL_ALL_F:	// General purpose PLL, function overwrites
					wm8804_pllnew(WM8804_PLL_NORMAL | WM8804_PLL_FORCE);
				break;
				case SPDIF_WM_PLL_192_F:	// 192ksps PLL, function overwrites
					wm8804_pllnew(WM8804_PLL_192 | WM8804_PLL_FORCE);
				break;
				case SPDIF_WM_PLL_TOGGLE:	// Toggle PLL status, no forcing
					wm8804_pllnew(WM8804_PLL_TOGGLE);
				break;
				case SPDIF_WM_SPDSTAT:
					temp_spdif_u8 = wm8804_read_byte(0x0C);	// SPDstat register
					print_dbg_char_char('.');
					print_dbg_char_hex(temp_spdif_u8);
				break;
				
			}
			
			if ( (spdif_cmd != SPDIF_CMD_MUSTACK) && (spdif_cmd != SPDIF_CMD_IDLE) ) {
				spdif_cmd = SPDIF_CMD_MUSTACK;	// Command has been executed
			}
		#endif
		// End of command handler
		


/*
// It's possible to run spdif handler here, but it's very tight in the time domain. Two consumers probably can't coexist at given MCU speed and algorithm complexity
		if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio) {
			mobo_handle_spdif(32);					// Polling code. UAC2 uses 32-bit data - moved here from uac2_dat.c
		}
		else if (feature_get_nvram(feature_image_index) == feature_image_uac2_audio) {
			mobo_handle_spdif(24);					// Polling code. UAC1 uses 24-bit data - moved here from uac1_dat.c
		}
*/
		
		poll_counter ++;							// Don't always do everything
		
		// while playing, got interrupt. Could be loss of link (monitored faster on pin) or TRANS_ERR (only visible on interrupt)

		// 		if (wm8804_read_byte(0x0B) & 0x08) {	// TRANS_ERR bit. This read clears interrupt status but WM8804 may be quick to set it again
		// try wm8804_pllnew(WM8804_PLL_TOGGLE);
		// then scan inputs starting with last known good input	
				
		// USB has assumed control, power down WM8804 if it was on
		if (input_select == MOBO_SRC_UAC2) {
			
			
			#ifdef FEATURE_SPDIF_CMD
				if (spdif_enable_state_machine) {
			#else
				if (1) {
			#endif
					if (spdif_rx_status.powered == 1) {
						spdif_rx_status.powered = 0;
						wm8804_sleep();
					}
				} // ifdef FEATURE_SPDIF_CMD .. if()
		}
				
		// USB does NOT have control. So consider what is going on with WM8804
		else {

			// (Trying to be) playing music from WM8804 - is everything OK?
			if ( (input_select == MOBO_SRC_SPDIF0) || (input_select == MOBO_SRC_TOSLINK1) || (input_select == MOBO_SRC_TOSLINK0) ) {
				mustgive = 0;									// Not ready to give up playing audio just yet
						
				// Poll two silence detectors, WM8804 and buffer transfer code
				if ( (SPDIF_IS_SILENT()) || (gpio_get_pin_value(WM8804_ZERO_PIN) == 1) ) {	// Either own SW based test and RX chip's zero detect
//				if ( (SPDIF_IS_SILENT()) || (0)                                        ) {	// Only own SW based test

					#ifdef FEATURE_SPDIF_CMD					// Log source of mustgive
						print_dbg_char('s');
						if (SPDIF_IS_SILENT()) {
							print_dbg_char('C');				// CPU sees silence
						}
						if (gpio_get_pin_value(WM8804_ZERO_PIN) == 1) {
							print_dbg_char('R');				// Receiver sees silence
						}
					#endif

					scanmode = WM8804_SCAN_FROM_NEXT + 0x05;	// Start scanning from next channel. Run up to 5x4 scan attempts
					mustgive = 1;

					#ifdef LOOSE_SIGNAL_LED									// Indicate startup with WHITE-RED-BLUE-(WHITE)
						mobo_led(FLED_RED);
						vTaskDelay(1000);
						mobo_led(FLED_WHITE);
						vTaskDelay(1000);
						mobo_led(FLED_RED);
						vTaskDelay(1000);
					#endif
				} // Silence not detected

				// Poll lost lock pin
				if (gpio_get_pin_value(WM8804_CSB_PIN) == 1) {	// Lost lock
					
					#ifdef FEATURE_SPDIF_CMD					// Log source of mustgive
						print_dbg_char('L');
					#endif
					
					// Count to more than one error?
					scanmode = WM8804_SCAN_FROM_NEXT + 0x05;	// Start scanning from next channel. Run up to 5x4 scan attempts
					mustgive = 1;
					
					#ifdef LOOSE_SIGNAL_LED
						mobo_led(FLED_RED);
						vTaskDelay(1000);
						mobo_led(FLED_GREEN);
						vTaskDelay(1000);
						mobo_led(FLED_RED);
						vTaskDelay(1000);
					#endif
				}

				// Poll interrupt pin
				if  (gpio_get_pin_value(WM8804_INT_N_PIN) == 0) {
					wm8804_int = wm8804_read_byte(0x0B);		// Read and clear interrupts

					#ifdef FEATURE_SPDIF_CMD
						print_cpu_char('!');					// Log source of mustgive below
						print_cpu_char_hex(wm8804_int);
					#endif

					#ifdef FEATURE_SPDIF_CMD
						if (spdif_enable_state_machine) {
					#else
						if (1) {
					#endif

							if (wm8804_int & 0x08) {					// Transmit error bit -> Try same channel next, with inverted PLL setting
								wm8804_pllnew(WM8804_PLL_TOGGLE);
								scanmode = WM8804_SCAN_FROM_PRESENT + 0x05;	// Start scanning from same channel. Run up to 5x4 scan attempts
								mustgive = 1;

								#ifdef LOOSE_SIGNAL_LED
								mobo_led(FLED_RED);
								vTaskDelay(1000);
								mobo_led(FLED_BLUE);
								vTaskDelay(1000);
								mobo_led(FLED_RED);
								vTaskDelay(1000);
								#endif
							}

						} // ifdef FEATURE_SPDIF_CMD .. if()
					
				}
				
				// Sometimes poll sample rate - dude, this happens a lot!
				if ( (poll_counter & 0x0003) == 0) {				// Once every 80ms while playing check if sample rate is correct with configTSK_WM8804_PERIOD = 200
					freq = mobo_srd();

					// If srd() returned a valid frequency that is different from the one we believe we're at, do something!					
					if ( ( (freq == FREQ_44) || (freq == FREQ_48) || (freq == FREQ_88) || (freq == FREQ_96) || (freq == FREQ_176) || (freq == FREQ_192) ) && (freq != spdif_rx_status.frequency) ) {
						// wm8804_pllnew(WM8804_PLL_TOGGLE);		// No PLL toggle -> quick to return to present setting
						scanmode = WM8804_SCAN_FROM_PRESENT + 0x05;	// Start scanning from same channel to prevent consequences of false detects. Run up to 5x4 scan attempts

						#ifdef FEATURE_SPDIF_CMD
							print_cpu_char('f');					// Log source of mustgive
							print_dbg_char_hex(freq >> 10);			// Output is hex ~ksps
						#endif

						mustgive = 1;

						#ifdef LOOSE_SIGNAL_LED
							mobo_led(FLED_RED);
							vTaskDelay(1000);
							mobo_led(FLED_CYAN);
							vTaskDelay(1000);
							mobo_led(FLED_RED);
							vTaskDelay(1000);
						#endif
					}
				}
						
				// Give away control?
				if (mustgive) {
					
					#ifdef FEATURE_SPDIF_CMD
						if (spdif_enable_state_machine) {
					#else
						if (1) {
					#endif
							wm8804_mute();
						} // ifdef FEATURE_SPDIF_CMD .. if()
		
					if (input_select != MOBO_SRC_NONE) {		// Always directly preceding give for RT reasons
						if (xSemaphoreGive(input_select_semphr) == pdTRUE) {
	//						Added to pdca disable code, keep it here for good measure
							mobo_stop_spdif_tc();					// Disable spdif receive timer/counter
							mobo_clear_dac_channel();				// Leave the DAC buffer empty as we check out
							spdif_rx_status.silence_SPDIF = SILENCE_SPDIF_INIT; 
							print_dbg_char('}');					// WM8804 gives
							print_dbg_char('\n');					// WM8804 gives

							// Report to cpu and debug terminal
							print_cpu_char(CPU_CHAR_IDLE);
							#ifdef HW_GEN_SPRX
								mobo_led_select(FREQ_NOCHANGE, MOBO_SRC_NONE);	// User interface NO-channel indicator
							#endif
							input_select = MOBO_SRC_NONE;			// Do this LATE! Indicate WM may take over control
						}
						else {
							print_dbg_char('*');
							print_dbg_char('e');
						}
					} // if (input_select != MOBO_SRC_NONE)
				}
			}

			// USB and WM8804 have given away active control, see if WM8804 can grab it
			if (input_select == MOBO_SRC_NONE) {
				if (spdif_rx_status.powered == 0) {

					#ifdef FEATURE_SPDIF_CMD
						if (spdif_enable_state_machine) {
					#else
						if (1) {
					#endif
							wm8804_init();								// WM8804 was probably put to sleep before this. Hence re-init
							spdif_rx_status.powered = 1;
						} // ifdef FEATURE_SPDIF_CMD .. if()

				}

				// RXMODFIX: Newly enabled WM8804 takes much longer time to lock on to audio stream!

				else {											// Don't start scanning immediately after power-on
					channel = spdif_rx_status.channel;			// Use receiver scan history if it is of any use
					
					#ifdef FEATURE_SPDIF_CMD
						if (spdif_enable_state_machine) {
					#else
						if (1) {
					#endif
							wm8804_scannew(&channel, &freq, scanmode);
						} // ifdef FEATURE_SPDIF_CMD .. if()
					
					if ( (freq != FREQ_TIMEOUT) && (freq != FREQ_INVALID) && (channel != MOBO_SRC_NONE)) {
						wm8804_read_byte(0x0B);					// Clear interrupts for good measure
								
						spdif_rx_status.channel = channel;
						spdif_rx_status.frequency = freq;
						spdif_rx_status.powered = 1;			// Written above
						
						// async spdif start: Wait for verified non-zero audio and then take!!
								
						// Take semaphore, update status if that went well
						if (input_select == MOBO_SRC_NONE) {				// Always directly preceding take
							if (xSemaphoreTake(input_select_semphr, 10) == pdTRUE) {	// Re-take of taken semaphore returns false
								input_select = channel;						// Owning semaphore we may write to master variable input_select and take control of hardware
								spdif_rx_status.channel = channel;
								spdif_rx_status.frequency = freq;
								print_dbg_char('{');						// WM8804 takes

								// Trying this as only setup site in wm8804 code...
//								mobo_xo_select(spdif_rx_status.frequency, input_select);
//								mobo_clock_division(spdif_rx_status.frequency);
								samples_per_package_min = (spdif_rx_status.frequency >> 12) - (spdif_rx_status.frequency >> 14); // 250us worth of music +- some slack
								samples_per_package_max = (spdif_rx_status.frequency >> 12) + (spdif_rx_status.frequency >> 14);
								must_init_xo = TRUE;
								// End only site of setup code

								// Report to cpu and debug terminal
								switch (input_select) {
									case MOBO_SRC_SPDIF0:
										print_cpu_char(CPU_CHAR_SPDIF0);	// SPDIF0
									break;
									case MOBO_SRC_SPDIF1:
										print_cpu_char(CPU_CHAR_SPDIF1);	// SPDIF1
									break;
									case MOBO_SRC_TOSLINK0:
										print_cpu_char(CPU_CHAR_TOSLINK0);	// TOSLINK0
									break;
									case MOBO_SRC_TOSLINK1:
										print_cpu_char(CPU_CHAR_TOSLINK1);	// TOSLINK1
									break;
									default:
										print_cpu_char(CPU_CHAR_SRC_DEF);	// Inform CPU (when present)
									break;
								}
							
								// Enable audio, configure clocks (and report), but only if needed
								wm8804_unmute();							// No longer including LED change on this TAKE event
							
								spdif_rx_status.silence_SPDIF = SILENCE_SPDIF_LIMIT - SILENCE_SPDIF_SCANNING; // Detector counts up to SILENCE_SPDIF_LIMIT during 200ms of linking time
							}
							else {
								print_dbg_char('*');
								print_dbg_char('f');
							}
						} // if (input_select = MOBO_SRC_NONE)
					} // Scan success
				}
			} // Done processing no selected input source
					
		} // Done considering what is happening to WM8804
	
	
	
	} // while (1)
} // wm8804_task


// Start up the WM8804
void wm8804_init(void) {

// Arbitrary startup delay ATD
//	static uint8_t initial = 1;

	spdif_rx_status.channel = MOBO_SRC_NONE;	// Start from a known state on wakeup

	if (wm8804_mclk_out(WM8804_MCLK_TEST) == WM8804_MCLK_ENABLE) {
		// With CLKOUT
		wm8804_write_byte(0x08, 0x70);	// 7:0 CLK2, 6:1 auto error handling disable, 5:1 zeros@error, 4:1 CLKOUT enable, 3:0 CLK1 out, 2-0:000 ignored // WM8804 rewrite
	}
	else if (wm8804_mclk_out(WM8804_MCLK_TEST) == WM8804_MCLK_DISABLE) {
		// Without CLKOUT export. (MCLK pin is not connected in SPRX D and E)
		wm8804_write_byte(0x08, 0x60);	// 7:0 CLK2, 6:1 auto error handling disable, 5:1 zeros@error, 4:0 CLKOUT disable, 3:0 CLK1 out, 2-0:000 ignored // WM8804 rewrite
	}

	wm8804_write_byte(0x1C, 0xCE);	// 7:1 I2S alive, 6:1 master, 5:0 normal pol, 4:0 normal, 3-2:11 or 10 24 bit, 1-0:10 I2S ? CE or CA ? // WM8804 same

//	wm8804_write_byte(0x1D, 0xC0);	// 7 SPD_192K_EN = 1, Change 6:1, disable data truncation, run on 24 bit I2S // WM8804 ignores bit 5
	wm8804_write_byte(0x1D, 0b11001000); // Same as above, with CONT enabled

	wm8804_write_byte(0x18, 0x07);	// 3:0 GPO1=UNLOCK (=SPIO_05_GPO1, PX15, WM8804_CSB_PIN) // WM8804 ported

	wm8804_write_byte(0x1A, 0x00);  // 3:0 GPO2=INT_N (=SPIO_04_GPO2, PA04, WM8804_INT_N_PIN) Retained WM8805 comment: "OK with initial software set to 1?" // WM8804 ported

	wm8804_write_byte(0x17, 0x0C);	// 3:0 GPO0=ZEROFLAG (=SPIO_00_SPO0, PX54, WM8804_ZERO_PIN=WM8804_SWIFMODE_PIN) // WM8804 ported

	wm8804_write_byte(0x0A, 0b11100100);	// REC_FREQ:mask (broken in wm!), DEEMPH:ignored, CPY:ignored, NON_AUDIO:active // WM8804 same
											// TRANS_ERR:active, CSUD:ignored, INVALID:active, UNLOCK:active

	wm8804_read_byte(0x0B);			// Clear interrupts // WM8804 same

	wm8804_write_byte(0x1E, 0x1F);	// Power down 7-6:0, 5:0 OUT, 4:1 _IF, 3:1 _OSC, 2:0 _TX, 1:1 _RX, 0:1 _PLL

// Arbitrary startup delay ATD
//	if (initial == 1) {
//		vTaskDelay(1000);
//		initial = 0;
//	}

#ifdef FEATURE_ADC_EXPERIMENTAL
	if (I2S_consumer == I2S_CONSUMER_NONE) {				// No other consumers? Enable DMA - ADC_site with what sample rate??
		// Enable CPU's processing of produced data
		// This is needed for the silence detector
		mobo_clear_adc_channel();							// Clear buffer before pdca starts filling it
		AK5394A_pdca_rx_enable(FREQ_INVALID);				// Start up without caring about I2S frequency or synchronization - doesn't happen in FMADC code
		mobo_start_spdif_tc(FREQ_192);						// Fast interrupts -> small packets. That is presumably better than too long packets which may overrun buffer lengths
		// FEATURE_ADC_EXPERIMENTAL spdif timer/counter is not needed for pure ADC -> USB on a large buffer. But it is needed if spdif loopback is introduced at some time. If there is enough runtime in uac2_dat.c, that is
	}
//	I2S_consumer |= I2S_CONSUMER_DAC;						// DAC state machine doesn't really subscribe to incoming I2S, it only scans for it...
#else
	mobo_clear_adc_channel();								// Clear buffer before pdca starts filling it
	AK5394A_pdca_rx_enable(FREQ_INVALID);					// Start up without caring about I2S frequency or synchronization
	mobo_start_spdif_tc(FREQ_192);							// Fast interrupts -> small packets. That is presumably better than too long packets which may overrun buffer lengths
#endif

}

// Turn off wm8804, why can't we just run init again? 
void wm8804_sleep(void) {
#ifdef FEATURE_ADC_EXPERIMENTAL
	I2S_consumer &= ~I2S_CONSUMER_DAC;						// DAC is no longer subscribing to I2S data
	if (I2S_consumer == I2S_CONSUMER_NONE) {				// No other consumers? Disable DMA
		mobo_stop_spdif_tc();								// Not using spdif rx timer when spdif source is disabled
		pdca_disable(PDCA_CHANNEL_SSC_RX);					// Disable I2S reception at MCU's ADC port
		pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
	}
#else
	mobo_stop_spdif_tc();									// Not using spdif rx timer when spdif source is disabled
	pdca_disable(PDCA_CHANNEL_SSC_RX);						// Disable I2S reception at MCU's ADC port
	pdca_disable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
#endif

	wm8804_write_byte(0x1E, 0x1F);	// Power down 7-6:0, 5:0 OUT, 4:1 _IF, 3:1 _OSC, 2:1 _TX, 1:1 _RX, 0:1 _PLL // WM8804 same
	mobo_SPRX_input(MOBO_SRC_NONE);	// For the time being (all SPRX revisions until F) select the unconnected SPDIF1 input. On rev. E that powers down the three assembled inputs
}


// Course detection of AC vs. DC on SPDIF input lines
// All hardware must support strap from U1:13 to U6:CP via R117 on rev. B and onwards. Default on Rev. E and onwards
// This used to be code switch HW_GEN_SPRX_PATCH_01
uint8_t wm8804_live_detect(void) {
	#define WM8804_SPDIF_LIVE_COUNT	0x20				// Detection takes about 50µs - ESD test: increase this number?
	uint8_t counter = WM8804_SPDIF_LIVE_COUNT;
	uint8_t chx = 0;

	// Time consumption of polling vs. 3-pulse period on slowest 44.1 ksps input. Must count for more than a 3-pulse period!
	gpio_set_gpio_pin(AVR32_PIN_PB04);					// Count enable

	// Poll SPDIF/TOSLINK data signal SPDIF_RX_CNT a number of times. Only bother with one of them in shared counter
	while (counter--) {
		if (gpio_get_pin_value(AVR32_PIN_PX16) == 1) {	// PCB patch from MUX output to net SPDIF0_TO_MCU / input MOBO_SRC_SPDIF0
			chx++;
		}
	}
	gpio_clr_gpio_pin(AVR32_PIN_PB04);					// Count disable

	// Report Live / Dead as binary code
	// A static SPDIF signal means the counter is either at 0 or at full value
	if ( (chx != 0) && (chx != WM8804_SPDIF_LIVE_COUNT) )
		return 1;
	else
		return 0;
}


// The start of a new automated input scanner. Doesn't detect silence
void wm8804_scannew(uint8_t *channel, uint32_t *freq, uint8_t mode) {
	uint8_t max_attempts = (mode & 0x0F) * 4;	// Up to 60 attempts before giving up
	uint8_t program = (mode & 0xF0);			// Which scanning sequence to use
	uint8_t program_index = 0;					// Start at the beginning of a program sequence
	uint8_t attempts = 0;	
	uint32_t temp_freq = FREQ_TIMEOUT;			// Valid frequency not yet detected
	#define SCAN_PROGRAM_LENGTH	3
	uint8_t temp_program[SCAN_PROGRAM_LENGTH];
	
	// No valid channel given, start scanning from default or user selection
	if ( (*channel != MOBO_SRC_TOSLINK0) && (*channel != MOBO_SRC_TOSLINK1) && (*channel != MOBO_SRC_SPDIF0) ) {
		*channel = spdif_rx_status.preferred_channel;	// Populated with default value or user override
	}
	
	// Relate to only one channel (user decision)
	if (program == WM8804_SCAN_ONE) {
		temp_program[0] = *channel;
		temp_program[1] = *channel;
		temp_program[2] = *channel;
	}
	// Start at specified channel, scan consecutives
	else if (program == WM8804_SCAN_FROM_PRESENT) {
		temp_program[0] = *channel;
		temp_program[1] = *channel+1;
		temp_program[2] = *channel+2;
	}
	// Start one channel beyond the specified one
	else {
		temp_program[0] = *channel+1;
		temp_program[1] = *channel+2;
		temp_program[2] = *channel+3;
	}
	
	// Sanitize content of program memory
	if (temp_program[0] > MOBO_SRC_HIGH) {
		temp_program[0] = temp_program[0] - MOBO_SRC_HIGH + MOBO_SRC_LOW - 1;
	}
	if (temp_program[1] > MOBO_SRC_HIGH) {
		temp_program[1] = temp_program[1] - MOBO_SRC_HIGH + MOBO_SRC_LOW - 1;
	}
	if (temp_program[2] > MOBO_SRC_HIGH) {
		temp_program[2] = temp_program[2] - MOBO_SRC_HIGH + MOBO_SRC_LOW - 1;
	}

	
	while (attempts++ < max_attempts) {			// Success causes function termination mid-loop
		temp_freq = wm8804_inputnew(temp_program[program_index]);	// Check if selected channel from program is online
		if ( (temp_freq != FREQ_PLLMISS) && (temp_freq != FREQ_TIMEOUT) && (temp_freq != FREQ_INVALID) ) {
			*channel = temp_program[program_index];	// Tell calling function which channel works
			*freq = temp_freq;					// ... and its sample rate
			
//			print_dbg_char_hex( (uint8_t)(temp_freq/1000) );
			return;
		}
		else if (temp_freq == FREQ_PLLMISS) {	// Linkup but PLL mismatch: try same channel again after toggling PLL setting
			
			#ifdef FEATURE_SPDIF_CMD
				if (spdif_enable_state_machine) {
			#else
				if (1) {
			#endif
					wm8804_pllnew(WM8804_PLL_TOGGLE);
				} // ifdef FEATURE_SPDIF_CMD .. if()

		}
		
		else {									// Select a new channel to try
			program_index++;
			if (program_index == SCAN_PROGRAM_LENGTH) {
				program_index = 0;
			}
		}
	}
	
	// while loop above didn't terminate with success, return failure	
	*channel = MOBO_SRC_NONE;					// No valid channel
	*freq = temp_freq;							// The (failing!) freq reported last by wm8804_inputnew()
	return;	
}


// Select input channel of the multiplexer preceding WM8804. NB: this is outside the scope of the chip itself!
// Naming convention: TOSLINK0 on schematic is TOSLINK1 in code etc.
uint32_t wm8804_inputnew(uint8_t input_sel) {
	uint8_t link_detect = 0;
	uint8_t link_attempts = 0;
	uint8_t trans_err_detect = 0;
	uint32_t freq; 
	uint32_t clkdiv_temp = 0;


// #ifdef HW_GEN_SPRX_PATCH_01
// PATCH_01 of RXmod_t1_A and RXmod_t1_C will multiplex first and then check if MUX output is alive. 
// This saves two flip-flops and a shitload of routing
// PATCH_01 consists of:
// FIX: remove R117 from U3
// FIX: patch U1:13 to R11 remaining side
// FIX: remove R116
// If this all works, change code for fourth digital input. Rewrite variables to match schematic

	// If given input is not alive, terminate
	mobo_SPRX_input(input_sel);			// Hardware MUX control
	vTaskDelay(1);						// How long time does this take? 1 -> 0.1ms - permit time to turn on detection circuit. Input selector code doubles as power enable starting with PCBA rev. E

	if ( !(wm8804_live_detect()) ) {
		//	Set all frequencies of input_sel to NORMAL = we know nothing about this input's rate relative to own XOs
		mobo_rate_storage(0, input_sel, SI_NORMAL, RATE_CH_INIT);	
		mobo_SPRX_input(MOBO_SRC_NONE);	// Disable power to input circuits until another one is enabled. Was: MOBO_SRC_SPDIF0
		return (FREQ_INVALID);
	}
	// If given input is alive, do things
	else {
		if (wm8804_mclk_out(WM8804_MCLK_TEST) == WM8804_MCLK_ENABLE) {
			// With CLKOUT
			wm8804_write_byte(0x08, 0x30);			// 7:0 CLK2, 6:0 auto error handling enable, 5:1 zeros@error, 4:1 CLKOUT enable, 3:0 CLK1 out, 2-0:0 no RX mux in WM8804
		}
		else if (wm8804_mclk_out(WM8804_MCLK_TEST) == WM8804_MCLK_DISABLE) {
			// Default: without CLKOUT export. (MCLK pin is not connected in SPRX D and E)
			wm8804_write_byte(0x08, 0x20);			// 7:0 CLK2, 6:0 auto error handling enable, 5:1 zeros@error, 4:0 CLKOUT disable, 3:0 CLK1 out, 2-0:0 no RX mux in WM8804
		}
		
		wm8804_write_byte(0x1E, 0x04);			// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:0 RX, 0:0 PLL // WM8804 same bit use, not verified here

		// The time it takes to verify a channel is very much different when the WM8804 is cold (up to 300ms seen in measurements) vs warm (70-100ms)
		// RXMODFIX Do something about that with slow scans if fast scans fail or something

		while (link_attempts++ < wm8804_LINK_MAX_ATTEMPTS) {		// Repeat until timeout

			// Check UNLOCK bit if everything is OK and we can leave this function
			//		if ( (wm8804_read_byte(0x0C) & 0x40) == 0 ) {	// UNLOCK bit. Does much the same job but takes a few µs longer than GPIO read. Not fully verified in 192ksps
			if (gpio_get_pin_value(WM8804_CSB_PIN) == 0) {	// Got link!
			
				// Make a log of how many poll cycles were needed to establish link
				if (link_attempts > link_attempts_max) {
					link_attempts_max = link_attempts;
				}
				if (link_attempts < link_attempts_min) {
					link_attempts_min = link_attempts;
				}
			
				if (link_detect++ == wm8804_LINK_DETECTS_OK-1) {	// We have a valid link!
					freq = mobo_srd();						// Now that we have link, measure the received sample rate
					clkdiv_temp = wm8804_clkdivnew(freq);	// Compare to WM8804's PLL setting and frequency detector and set up clock division for MCLK export
					
					if (clkdiv_temp == WM8804_CLK_SUCCESS) {	
				
						// RXMODFIX Also power cycle PLL? Also verify that detected sample rate matches present PLL configuration?					
						return freq;						// Got link enough times, wm8804_srd() and WM8804 agree on clock configuration -> return detected frequency
					}
					else if (clkdiv_temp == WM8804_CLK_PLLMISS) {
						return FREQ_PLLMISS;
					}
					
				}
			}
			else {											// No link, temporary, glitch or permanent. Forget detections until now
				link_detect = 0;							// Consecutive good detects are needed!
			}
		
			// Check TRANS_ERR bit to determine if we must change PLL settings
			if (wm8804_read_byte(0x0B) & 0x08) {	// TRANS_ERR bit. This read clears interrupt status but WM8804 may be quick to set it again
				if (trans_err_detect++ == wm8804_TRANS_ERR_FAILURE-1) {

					#ifdef FEATURE_SPDIF_CMD
						print_cpu_char('%');
						if (spdif_enable_state_machine) {
					#else
						if (1) {
					#endif
							wm8804_pllnew(WM8804_PLL_TOGGLE);
						} // ifdef FEATURE_SPDIF_CMD .. if()

					trans_err_detect = 0;					// New try with new setting! - dependant on spdif_enable_state_machine?

				}
			}
			else {											// No link, temporary, glitch or permanent. Forget detections until now
				trans_err_detect = 0;						// Consecutive error detects are needed! We assume the interrupt generator is faster than vTaskDelay() below
			}
		
			vTaskDelay(50);									// How long time does this take? 50 -> 5.00ms
		}

		//	print_dbg_char('!');
	
		return (FREQ_TIMEOUT);					// Couldn't get lock = timeout Maybe return FREQ_INVALID for unstable PLL?
	}	// End of do-stuff-if-alive

}


// Report link statistics
void wm8804_linkstats(void) {
	print_dbg_char('m');
	print_dbg_char_hex(link_attempts_min);
	print_dbg_char('M');
	print_dbg_char_hex(link_attempts_max);
	print_dbg_char('\n');
}


// Pll setting for WM8804
void wm8804_pllnew(uint8_t pll_sel) {
	static uint8_t pll_sel_prev = WM8804_PLL_NORMAL;	// Chip default value
	uint8_t dev_data[5];
	
	// Are we forcing an update?
	if (pll_sel & WM8804_PLL_FORCE) {	
		pll_sel_prev = WM8804_PLL_FORCE;
		pll_sel = pll_sel & (~WM8804_PLL_FORCE);		// And after bitwise invert with the mask
		// print_dbg_char_hex(pll_sel);
		// print_dbg_char_hex(pll_sel_prev);
	}

	// Ignore no change 
	if (pll_sel == pll_sel_prev) {
//		print_dbg_char('.');				// No change -> do nothing
	}
	else {									// Implement change
		if (pll_sel == WM8804_PLL_TOGGLE) {
			if (pll_sel_prev == WM8804_PLL_NORMAL) {
				pll_sel = WM8804_PLL_192;
			}
//			else if (pll_sel_prev == WM8804_PLL_192) {
			else {							// Force and toggle shouldn't be used. It defaults to WM8804_PLL_NORMAL
				pll_sel = WM8804_PLL_NORMAL;
			}
		}


//LeavePLL			wm8804_write_byte(0x1E, 0x07);		// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:1 _RX, 0:1 _PLL // WM8804 same bit use, not verified here NB: disabling PLL before messing with it

		// Default PLL setup for 44.1, 48, 88.2, 96, 176.4
		if (pll_sel == WM8804_PLL_NORMAL) {
			#ifdef FEATURE_SPDIF_CMD
				print_dbg_char('_'); 
			#endif

			dev_data[0] = 0x03;
			dev_data[1] = 0x21; // 0x03 data PLL_K[7:0] 21
			dev_data[2] = 0xFD; // 0x04      PLL_K[15:8] FD
			dev_data[3] = 0x36; // 0x05      7:0 , 6:0, 5-0:PLL_K[21:16] 36
			dev_data[4] = 0x07; // 0x06      7:0 , 6:0 , 5:0 , 4:0 Prescale/1 , 3-2:PLL_N[3:0] 7
			wm8804_multiwrite(5, dev_data);

/*			Old single-write code
			wm8804_write_byte(0x03, 0x21);	// PLL_K[7:0] 21
			wm8804_write_byte(0x04, 0xFD);	// PLL_K[15:8] FD
			wm8804_write_byte(0x05, 0x36);	// 7:0 , 6:0, 5-0:PLL_K[21:16] 36
			wm8804_write_byte(0x06, 0x07);	// 7:0 , 6:0 , 5:0 , 4:0 Prescale/1 , 3-2:PLL_N[3:0] 7
*/			
			
			spdif_rx_status.pllmode = pll_sel; 
		}

		// Special PLL setup for 192
		else if (pll_sel == WM8804_PLL_192) {	// PLL setting 8.192
			#ifdef FEATURE_SPDIF_CMD 
				print_dbg_char('#');
			#endif

			dev_data[0] = 0x03;
			dev_data[1] = 0xBA; // 0x03 data PLL_K[7:0] BA
			dev_data[2] = 0x49; // 0x04      PLL_K[15:8] 49
			dev_data[3] = 0x0C; // 0x05      7:0,  6:0, 5-0:PLL_K[21:16] 0C
			dev_data[4] = 0x08; // 0x06      7: , 6: , 5: , 4: , 3-2:PLL_N[3:0] 8
			wm8804_multiwrite(5, dev_data);
			
/*			Old single-write code
			wm8804_write_byte(0x03, 0xBA);	// PLL_K[7:0] BA
			wm8804_write_byte(0x04, 0x49);	// PLL_K[15:8] 49
			wm8804_write_byte(0x05, 0x0C);	// 7:0,  6:0, 5-0:PLL_K[21:16] 0C
			wm8804_write_byte(0x06, 0x08);	// 7: , 6: , 5: , 4: , 3-2:PLL_N[3:0] 8
*/

			spdif_rx_status.pllmode = pll_sel;
		}
	
		wm8804_write_byte(0x1E, 0x04);		// 7-6:0, 5:0 OUT, 4:0 IF, 3:0 OSC, 2:1 _TX, 1:0 RX, 0:0 PLL

		pll_sel_prev = pll_sel;				// Record history
	}
}	// wm8804_pllnew()



// Old WM8805 results 
// Delays of:
// 400 / 1200	Poor cold start
// 600 / 1200	Warm start OK. Cold start failed
// 600 / 3000	Warm start OK. Cold start OK
// 600 / 2000	Warm start OK. Cold start OK
// 600 / 1500	Warm start OK. Cold start OK
 

// Set up WM8804 CLKOUTDIV so that CLKOUT is in the 22-24MHz range
// Compare expected frequency (typically measured by mobo_srd() to WM8804's internally registered frequency
// Also verify that PLL has been configured correctly
uint8_t wm8804_clkdivnew(uint32_t freq) {
	uint8_t temp;
	temp = wm8804_read_byte(0x0C);		// Read SPDSTAT
	temp = temp & 0x30;					// Consider bits 5-4
	
	
	if ( (spdif_rx_status.pllmode != WM8804_PLL_192) && (freq == FREQ_192) ) {
		return WM8804_CLK_PLLMISS;							// Mismatch between input freq and PLL configuration
	}
	
	if ( (spdif_rx_status.pllmode != WM8804_PLL_NORMAL) && ( (freq == FREQ_44) || (freq == FREQ_48) || (freq == FREQ_88) || (freq == FREQ_96) || (freq == FREQ_176) ) ) {
		return WM8804_CLK_PLLMISS;							// Mismatch between input freq and PLL configuration
	}
	

	if ( (freq == FREQ_44) || (freq == FREQ_48) ) {			// 44.1 or 48 from srd() AND... 
		if ( (temp == 0x20) || (temp == 0x30) )	{			// 44.1, 48, or 32 from chip
			wm8804_write_byte(0x07, 0x0C);					// 7:0 , 6:0, 5-4:MCLK=512fs , 3:1 MCLKDIV=1 , 2:1 FRACEN , 1-0:0
			return WM8804_CLK_SUCCESS;
		}
	}
	else if ( (freq == FREQ_88) || (freq == FREQ_96) ) {	// 88.2 or 96 from srd() AND...
		if (temp == 0x10) {									// 88.2 or 96 from chip
			wm8804_write_byte(0x07, 0x1C);					// 7:0 , 6:0, 5-4:MCLK=256fs , 3:1 MCLKDIV=1 , 2:1 FRACEN , 1-0:0
			return WM8804_CLK_SUCCESS;
		}
	}
	else if ( (freq == FREQ_176) || (freq == FREQ_192) ) {	// 176.4 or 192 from srd() AND...
		if (temp == 0x00) {									// 192 from chip, NB: 176.4 not described in datasheet!
			wm8804_write_byte(0x07, 0x2C);					// 7:0 , 6:0, 5-4:MCLK=128fs , 3:1 MCLKDIV=1 , 2:1 FRACEN , 1-0:0
			return WM8804_CLK_SUCCESS;
		}
	}
	
	return WM8804_CLK_FAILURE;								// Mismatch between input freq and WM8804's freq
}


// Mute the WM8804 output
void wm8804_mute(void) {
	// Empty outgoing buffers if owned by WM8804 code
	if ( (input_select == MOBO_SRC_SPDIF0) || (input_select == MOBO_SRC_TOSLINK1) || (input_select == MOBO_SRC_TOSLINK0) ) {
		mobo_clear_dac_channel();
	}
													// Dedicated mute pin, leaves clocks etc intact
	mobo_i2s_enable(MOBO_I2S_DISABLE);				// Hard-mute of I2S pin, try to avoid using this hardware!
	dac_must_clear = DAC_MUST_CLEAR;				// Instruct uacX_device_audio_task.c to clear outgoing DAC data
}


// Un-mute the WM8804 - only called after succesful mutex take!!
void wm8804_unmute(void) {
	// For now, frequency changes totally mess up ADC_site
	
#ifdef FEATURE_ADC_EXPERIMENTAL
	if (I2S_consumer == I2S_CONSUMER_NONE) {					// No other consumers? Enable DMA - ADC_site with what sample rate??
		mobo_clear_adc_channel();								// Clear buffer before pdca starts filling it
		AK5394A_pdca_rx_enable(spdif_rx_status.frequency);		// New code to test for L/R swap
		mobo_start_spdif_tc(spdif_rx_status.frequency);			// Turn on the spdif timer/counter interrupt, not needed for pure ADC -> USB
	}
	I2S_consumer |= I2S_CONSUMER_DAC;							// DAC subscribes to incoming I2S
#else
	mobo_clear_adc_channel();									// Clear buffer before pdca starts filling it
	AK5394A_pdca_rx_enable(spdif_rx_status.frequency);			// New code to test for L/R swap
	mobo_start_spdif_tc(spdif_rx_status.frequency);				// Turn on the spdif timer/counter interrupt
#endif

	ADC_buf_I2S_IN = INIT_ADC_I2S;								// Force init of MCU's ADC DMA port. Until this point it is NOT detecting zeros..

	mobo_i2s_enable(MOBO_I2S_ENABLE);							// Hard-unmute of I2S pin. NB: we should qualify outgoing data as 0 or valid music!!
}



// Write multiple bytes to WM8804
uint8_t wm8804_multiwrite(uint8_t no_bytes, uint8_t *int_data) {
    uint8_t status = 0xFF;							// Far from 0 reported as I2C success

	// Wrap entire I2C transfer in semaphore, not just each I2C/twi function call
	if (xSemaphoreTake(I2C_busy_semphr, 0) == pdTRUE) {	// Re-take of taken semaphore returns false
		// Start of blocking code
		status = twi_write_out(WM8804_DEV_ADR, int_data, no_bytes);
		// End of blocking code

		if( xSemaphoreGive(I2C_busy_semphr) == pdTRUE ) {
		}
		else {
		}
	}
	else {
	}
	return status;
}


// Write a single byte to WM8804
uint8_t wm8804_write_byte(uint8_t int_adr, uint8_t int_data) {
    uint8_t dev_data[2];
    uint8_t status = 0xFF;							// Far from 0 reported as I2C success

	// Wrap entire I2C transfer in semaphore, not just each I2C/twi function call
	if (xSemaphoreTake(I2C_busy_semphr, 0) == pdTRUE) {	// Re-take of taken semaphore returns false

		// Start of blocking code
		dev_data[0] = int_adr;
		dev_data[1] = int_data;
		status = twi_write_out(WM8804_DEV_ADR, dev_data, 2);
		// End of blocking code

		if( xSemaphoreGive(I2C_busy_semphr) == pdTRUE ) {
		}
		else {
		}
	}
	else {
	}

	return status;
}


// Read a single byte from WM8804
uint8_t wm8804_read_byte(uint8_t int_adr) {
	uint8_t dev_data[1];
	
	// Wrap entire I2C transfer in semaphore, not just each I2C/twi function call
//	print_dbg_char('b');
	if (xSemaphoreTake(I2C_busy_semphr, 0) == pdTRUE) {	// Re-take of taken semaphore returns false
//		print_dbg_char('B');

		// Start of blocking code
		dev_data[0] = int_adr;
		if (twi_write_out(WM8804_DEV_ADR, dev_data, 1) == TWI_SUCCESS) {
			twi_read_in(WM8804_DEV_ADR, dev_data, 1);
		}
		else
			dev_data[0] = 0 ;	// Randomly chosen failure state
		
		// End of blocking code

		if( xSemaphoreGive(I2C_busy_semphr) == pdTRUE ) {
		}
		else {
		}
	}
	else {
	}

	return dev_data[0];
}

#endif  // HW_GEN_SPRX
