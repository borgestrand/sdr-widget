/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Management of the USB device mouse HID task.
 *
 * This file manages the USB device mouse HID task.
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
 * Additions and Modifications to ATMEL AVR32-SoftwareFramework-AT32UC3 are:
 *
 * Copyright (C) 2012 Borge Strand-Bergesen
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
 *
 * Modified by Alex Lee and sdr-widget team since Feb 2010.  Copyright General Purpose Licence v2.
 * Please refer to http://code.google.com/p/sdr-widget/
 */

//_____  I N C L U D E S ___________________________________________________


#include "conf_usb.h"
#if BOARD != EVK1104 && BOARD != SDRwdgtLite
#include "joystick.h"
#endif
#if BOARD == EVK1101
# include "lis3l06al.h"
#endif


#if USB_DEVICE_FEATURE == ENABLED

#include "board.h"
#ifdef FREERTOS_USED
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "usb_drv.h"
#include "gpio.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_specific_request.h"
#include "device_mouse_hid_task.h"
#include "Mobo_config.h"
#include "features.h"

// To access global input source variable
#include "device_audio_task.h"

#if LCD_DISPLAY			// Multi-line LCD display
#include "taskLCD.h"
#endif
//#include "taskEXERCISE.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________



//_____ D E C L A R A T I O N S ____________________________________________

// static U8 usb_state = 'r'; // BSB 20120718 unused variable, sane?
// static U8 ep_hid_rx; // BSB 20120718 unused variable, sane?
   static U8 ep_hid_tx;
//!
//! @brief This function initializes the hardware/software resources
//! required for device HID task.
//!
void device_mouse_hid_task_init(U8 ep_rx, U8 ep_tx)
{

#if BOARD == EVK1101
	// Initialize accelerometer driver
	acc_init();
#endif
//	ep_hid_rx = ep_rx; // BSB 20120718 unused variable, sane?
	ep_hid_tx = ep_tx;
#ifndef FREERTOS_USED
#if USB_HOST_FEATURE == ENABLED
	// If both device and host features are enabled, check if device mode is engaged
	// (accessing the USB registers of a non-engaged mode, even with load operations,
	// may corrupt USB FIFO data).
	if (Is_usb_device())
#endif  // USB_HOST_FEATURE == ENABLED
		Usb_enable_sof_interrupt();
#endif  // FREERTOS_USED

#ifdef FREERTOS_USED
	xTaskCreate(device_mouse_hid_task,
				configTSK_USB_DHID_MOUSE_NAME,
				configTSK_USB_DHID_MOUSE_STACK_SIZE,
				NULL,
				configTSK_USB_DHID_MOUSE_PRIORITY,
				NULL);
#endif  // FREERTOS_USED

	// Added BSB 20120718
	print_dbg("\nHID ready\n"); // usart is ready to receive HID commands!

	// Added BSB 20120719
#define HID2LCD					// Use LCD to debug incoming HID commands from uart

#ifdef HID2LCD					// Needed here? Including these lines seems to break functionality
//	lcd_q_init();
//	lcd_q_clear();
#endif


#if defined(HW_GEN_DIN10)
//	wm8805_init();				// Start up the WM8805 in a fairly dead mode
#endif


}



//!
//! @brief Entry point of the device mouse HID task management
//!
#ifdef FREERTOS_USED
void device_mouse_hid_task(void *pvParameters)
#else
void device_mouse_hid_task(void)
#endif
{
//  U8 data_length; // BSB 20120718 unused variable, sane?
//  const U8 EP_HID_RX = ep_hid_rx; // BSB 20120718 unused variable, sane?
  const U8 EP_HID_TX = ep_hid_tx;

  // BSB 20120810 HID variables moved up
  const U8 ReportByte0 = 0x01;	// Report ID doesn't change
  U8 ReportByte1 = 0;				// 1st variable byte of HID report
  U8 ReportByte2 = 0; 			// 2nd variable byte of HID report
  U8 ReportByte1_prev = 0;		// Previous ReportByte1
  char a = 0;						// ASCII character as part of HID protocol over uart
  char gotcmd = 0;				// Initially, no user command was recorded


#ifdef FREERTOS_USED
  portTickType xLastWakeTime;

  xLastWakeTime = xTaskGetTickCount();
  while (TRUE)
  {
    vTaskDelayUntil(&xLastWakeTime, configTSK_USB_DHID_MOUSE_PERIOD);

    // First, check the device enumeration state
    if (!Is_device_enumerated()) continue;
#else
    // First, check the device enumeration state
    if (!Is_device_enumerated()) return;
#endif  // FREERTOS_USED


// BSB 20120711: Debugging HID
/*

// Original code from Atmel commented out

       switch (usb_state){

       case 'r':
	   if ( Is_usb_out_received(EP_HID_RX)){
		   LED_Toggle(LED1);
		   Usb_reset_endpoint_fifo_access(EP_HID_RX);
		   data_length = Usb_byte_count(EP_HID_RX);
		   if (data_length > 2) data_length = 2;
		   usb_read_ep_rxpacket(EP_HID_RX, &usb_report[0], data_length, NULL);
		   Usb_ack_out_received_free(EP_HID_RX);
#if LCD_DISPLAY			// Multi-line LCD display
		   xSemaphoreTake( mutexQueLCD, portMAX_DELAY );
		   lcdQUEDATA.CMD = lcdPOSW;
	       xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
           lcdQUEDATA.CMD=lcdGOTO;
           lcdQUEDATA.data.scrnPOS.row = 3;
           lcdQUEDATA.data.scrnPOS.col = 14;
           xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
           lcdQUEDATA.CMD=lcdPUTH;
           lcdQUEDATA.data.aChar=usb_report[0];
           xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
           lcdQUEDATA.CMD=lcdPUTH;
           lcdQUEDATA.data.aChar=usb_report[1];
           xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
           lcdQUEDATA.CMD = lcdPOSR;
           xStatus = xQueueSendToBack( lcdCMDQUE, &lcdQUEDATA, portMAX_DELAY );
           xSemaphoreGive( mutexQueLCD );
#endif
           print_dbg("HID: report received\n");

		   usb_state = 't';
	   }
	   break;

       case 't':

       if( Is_usb_in_ready(EP_HID_TX) )
       {
    	  LED_Toggle(LED0);
          Usb_reset_endpoint_fifo_access(EP_HID_TX);
          
          //! Write report
          Usb_write_endpoint_data(EP_HID_TX, 8, usb_report[0]);
          Usb_write_endpoint_data(EP_HID_TX, 8, usb_report[1]);
          Usb_ack_in_ready_send(EP_HID_TX);
          usb_state = 'r';
       }
       break;
       }
*/


    // BSB 20120711: HID using uart

/*
 * HID protocol using uart
 *
 * baud rate = 57600 (to be lowered to accommodate bit-banging hardware)
 * data bits = 8, stop bits = 1 (probably, haven't messed with defaults in TeraTerm...)
 *
 * At reboot usb-i2s module reports "\nHID ready\n"
 *
 * MCU sends usb-i2s module the ASCII command "hXXYY" where 'h' is the letter 'h',
 * XX is ASCII encoded ReportByte1, YY is ASCII encoded ReportByte2. usb-i2s module
 * echos transmission. A USB HID report of 0x01, XX, YY is sent to HOST. After sending it,
 * usb-i2s module sends 'H' '\n' to the MCU over uart. If USB HID report fails, '-' '\n'
 * is sent to the MCU.
 *
 * Both key press ( (XX != 0) || (YY != 0) ) and key release (XX == YY == 0) must be sent
 * separately by the MCU. The HOST decodes this into short, long and double key presses
 *
 * See usb_descriptors.c: usb_hid_report_descriptor[USB_HID_REPORT_DESC] on how XX and YY
 * encode user buttons.
 *
 * See BasicAudioDevice-10.pdf for more info
 * See "12 Consumer" of http://stuff.mit.edu/afs/sipb/project/freebsd/head/share/misc/usb_hid_usages
 *
 * Bugs: At present it only works with YY == 00. Reworking HID report has not been successful.
 *
 */

    gotcmd = 0;												// No HID button change recorded yet

    uint8_t temp1, temp2;
    uint8_t input_select_wm8805_next = MOBO_SRC_SPDIF;		// Try SPDIF first
    uint8_t wm8805_pllmode = WM8805_PLL_NORMAL;				// Normal PLL setting at WM8805 reset
    uint8_t wm8805_muted = 1;								// Assume I2S output is muted
    U32 wm8805_freq = FREQ_TIMEOUT;							// Sample rate variables, no sample rate yet detected
    uint16_t wm8805_zerotimer = SILENCE_WM_LIMIT;			// Initially assume WM8805 is silent
    uint16_t wm8805_loudtimer = LOUD_WM_INIT;				// Initially assume WM8805 is silent

    while (gotcmd == 0) {

    	// These are test sequences. Move automated stuff to taskMoboCtrl.c

    	if (readkey()) {									// Check for an UART character command
            a = read_dbg_char(DBG_ECHO, RTOS_WAIT, DBG_CHECKSUM_NORMAL);	// UART character arrived, get it

            // HID debug
            if (a == 'h') {									// Wait until 'h' is entered to indicate HID activity
                ReportByte1 = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);	// Get 8 bits of hex encoded by 2 ASCII characters, with echo
                ReportByte2 = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);	// Get 8 bits of hex encoded by 2 ASCII characters, with echo
            	gotcmd = 1;									// HID received on UART gets sent regardless
            }

            // Debugging LEDs on HW_GEN_DIN10
            else if (a == 'l') {
            	mobo_led(read_dbg_char_hex(DBG_ECHO, RTOS_WAIT), read_dbg_char_hex(DBG_ECHO, RTOS_WAIT), read_dbg_char_hex(DBG_ECHO, RTOS_WAIT));
            	print_dbg_char('\n');
            }

            // Debugging WM8805 single write
            else if (a == 'w') {
            	// For some strange reason the two are swapped inside wm8805_write_byte() if
            	// parameters are entered as read_dbg_char_hex(). Very strange. The LED debugger is not yet tested for this..
            	temp1 = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);	// Internal address
            	temp2 = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);	// Data byte

            	print_dbg_char_hex(wm8805_write_byte(temp1, temp2));
            	print_dbg_char('\n');
			}

            // Debugging WM8805 single read, only valid for "read only" registers, and then with a twist...
            else if (a == 'r') {
            	print_dbg_char_hex( wm8805_read_byte(read_dbg_char_hex(DBG_ECHO, RTOS_WAIT)) );
            	print_dbg_char('\n');
			}

            // Start up the WM8805
            else if (a == 'z') {
            	wm8805_init();
				print_dbg_char('\n');
            }

            // Select WM8805 as I2S source, for now use only TOSLINK
            else if (a == 'W') {
            	wm8805_mute();								// Unmute and LED select will come after code detects lock
            	wm8805_muted = 1;

            	input_select = MOBO_SRC_TOSLINK;
            	wm8805_input(input_select);					// Is it good to do this late???

				wm8805_pllmode = WM8805_PLL_NORMAL;
				wm8805_pll(wm8805_pllmode);					// Is this a good assumption, or should we test its (not yet stable) freq?

				print_dbg_char('\n');
            }

            // Start hardware reset of WM8805
            else if (a == 's')
            	wm8805_reset(WM8805_RESET_START);

            // End hardware reset of WM8805
            else if (a == 't')
            	wm8805_reset(WM8805_RESET_END);

            // Change I2S source to USB, assume 44.1 UAC2
            else if (a == 'U') {
            	if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio)
            		input_select = MOBO_SRC_UAC1;
            	else
            		input_select = MOBO_SRC_UAC2;

				mobo_xo_select(current_freq.frequency, input_select);
				mobo_led_select(current_freq.frequency, input_select);
				wm8805_sleep();
            }

            else if (a == 'S')
				wm8805_sleep();

            else if (a == 'x') {
				wm8805_freq = mobo_srd();						// Print detected sample rate
				switch (wm8805_freq) {
					case FREQ_32:
						print_dbg_char_hex(0x32);
						break;
					case FREQ_44:
						print_dbg_char_hex(0x44);
						break;
					case FREQ_48:
						print_dbg_char_hex(0x48);
						break;
					case FREQ_88:
						print_dbg_char_hex(0x88);
						break;
					case FREQ_96:
						print_dbg_char_hex(0x96);
						break;
					case FREQ_176:
						print_dbg_char_hex(0x17);
						break;
					case FREQ_192:
						print_dbg_char_hex(0x19);
						break;
					default:
						print_dbg_char_hex(0x00);
						break;
				}
				wm8805_clkdiv();				// Configure MCLK division
				print_dbg_char('\n');
            }

            else if (a == 'm') {
				wm8805_unmute();
				print_dbg_char('\n');
            }

            else if (a == 'M') {
				wm8805_mute();
				print_dbg_char('\n');
            }

            else if (a == 'p') {
				wm8805_pllmode = WM8805_PLL_NORMAL;
				wm8805_pll(wm8805_pllmode);
				print_dbg_char('\n');
            }

            else if (a == 'P') {
				wm8805_pllmode = WM8805_PLL_192;
				wm8805_pll(wm8805_pllmode);
				print_dbg_char('\n');
            }

            // If you need the UART for something other than HID, this is where you interpret it!

    	} // if (readkey())

    	else { 											   	// GPIO pin _changes_ are sent to Host
/*
    		if ( (gpio_get_pin_value(PRG_BUTTON) == 0) ) {	// Check if Prog button is pushed down
				ReportByte1 = 0x04;							// Encode the Play/Pause HID command
				ReportByte2 = 0x00;							// This command is 0x00 until HID becomes more refined...
			}

			else if ( (gpio_get_pin_value(PRG_BUTTON) != 0) ) {	// Check if Prog button is released
				ReportByte1 = 0x00;							// Encode the button release HID command
				ReportByte2 = 0x00;							// This command is 0x00 until HID becomes more refined...
			}
*/

			// Add more pins to poll here!

			if (ReportByte1 != ReportByte1_prev) {			// Did we record a button change to send to Host?
				gotcmd = 1;
				ReportByte1_prev = ReportByte1;
			}


			/* NEXT:
			 * + Decide on which interrupts to enable
			 * + Do some clever bits with silencing
			 * + Prevent USB engine from going bonkers when playing on the WM (buffer zeros and send nominal sample rate...)
			 * - Test code base on legacy hardware
			 * - Check if WM is really 24 bits
			 * + Test SPDIF
			 * - Structure code away from mobo_config.c/h
			 * + Think about some automatic silence detecting software!
			 * - Long-term testing
			 */


/*			if (WM_IS_LOUD())
				mobo_led(FLED_DARK, FLED_YELLOW, FLED_DARK);
			else
				mobo_led(FLED_DARK, FLED_PURPLE, FLED_DARK);
*/
			// USB is halted, give control to WM8805 according to next source to be used
			if ( (USB_IS_SILENT()) && ( (input_select == MOBO_SRC_UAC1) || (input_select == MOBO_SRC_UAC2)  ) ) {
            	wm8805_mute();									// Unmute and LED select after code detects lock
            	wm8805_muted = 1;
            	playerStarted = PS_USB_OFF;						// Turn off USB audio

            	input_select = input_select_wm8805_next;		// Indicate to USB state machine to stay quiet if audio should arrive
            	if (input_select_wm8805_next == MOBO_SRC_TOSLINK) {	// Prepare to listen to other WM channel next time we're here
//					print_dbg_char('o');						// Debug must know which WM channel we're trying now
            		input_select_wm8805_next = MOBO_SRC_SPDIF;
            	}
            	else {
            		input_select_wm8805_next = MOBO_SRC_TOSLINK;
//					print_dbg_char('c');
            	}
//				print_dbg_char('\n');


				wm8805_zerotimer = SILENCE_WM_INIT;				// Assume it hasn't become silent yet at startup, give it time to figure out
				wm8805_loudtimer = LOUD_WM_INIT;				// WM8805 won't be playing music right away
            	wm8805_init();									// WM8805 was probably put to sleep before this. Hence re-init
            	wm8805_input(input_select);						// Is it good to do this late???
				wm8805_pllmode = WM8805_PLL_NORMAL;
				wm8805_pll(wm8805_pllmode);						// Is this a good assumption, or should we test its (not yet stable) freq?

//				print_dbg_char('W');
//				print_dbg_char('\n');
            }

			// Current WM8805 input is silent or unavailable
			if ( (WM_IS_SILENT()) && ( (input_select == MOBO_SRC_SPDIF) || (input_select == MOBO_SRC_TOSLINK)  ) ) {

				playerStarted = PS_USB_STARTING;				// Indicate that USB state machine may take control

				if (feature_get_nvram(feature_image_index) == feature_image_uac1_audio)
            		input_select = MOBO_SRC_UAC1;
            	else
            		input_select = MOBO_SRC_UAC2;

				mobo_xo_select(current_freq.frequency, input_select);	// Give USB the I2S control
				wm8805_sleep();

//				print_dbg_char('U');
//				print_dbg_char('\n');
			}

			// Check if WM8805 is able to lock and hence play music, only use when WM8805 is active
			if ( (wm8805_muted) && ( (input_select == MOBO_SRC_TOSLINK) || (input_select == MOBO_SRC_SPDIF)  ) ) {
//			if (wm8805_muted) {									// Try to unmute with qualified UNLOCK
				if ( (!wm8805_unlocked()) && (WM_IS_LOUD()) ) {	// Qualified lock with audio present
					wm8805_clkdiv();							// Configure MCLK division
					wm8805_unmute();							// Reconfigure I2S selection and LEDs
					wm8805_muted = 0;
//					print_dbg_char('!');
				}
			}

			// Polling interrupt monitor, only use when WM8805 is selected
			if ( (gpio_get_pin_value(WM8805_INT_N_PIN) == 0) && ( (input_select == MOBO_SRC_TOSLINK) || (input_select == MOBO_SRC_SPDIF)  ) ) {
//			if (gpio_get_pin_value(WM8805_INT_N_PIN) == 0) {	// There is an active low interrupt going on!
				temp1 = wm8805_read_byte(0x0B);					// Record interrupt status and clear pin

				if (wm8805_unlocked()) {						// Unlock
					if (!wm8805_muted) {
						wm8805_mute();
						wm8805_muted = 1;						// In any case, we're muted from now on.
					}

                	if (mobo_srd() == FREQ_192) {
//						print_dbg_char('P');
						wm8805_pllmode = WM8805_PLL_192;
					}
					else {
//						print_dbg_char('p');
						wm8805_pllmode = WM8805_PLL_NORMAL;
					}
                	wm8805_pll(wm8805_pllmode);					// Update PLL settings at any sample rate change!
                	vTaskDelay(3000);							// Let WM8805 PLL try to settle for 10ms
				}
/*
				// Interrupt reporting
				temp1 = wm8805_read_byte(0x0B);					// Record interrupt status and clear pin
				temp2 = wm8805_read_byte(0x0C);					// Record spdif status
				wm8805_freq = mobo_srd();						// Print detected sample rate

				print_dbg_char('I');							// Print recorded interrupt status
				print_dbg_char(':');
				print_dbg_char_bin(temp1);						// INTSTAT
				print_dbg_char(' ');
				print_dbg_char('S');
				print_dbg_char(':');
				print_dbg_char_bin(temp2);						// SPDSTAT
				print_dbg_char(' ');
				print_dbg_char('x');
				print_dbg_char(':');

				switch (wm8805_freq) {
					case FREQ_32:
						print_dbg_char_hex(0x32);
						break;
					case FREQ_44:
						print_dbg_char_hex(0x44);
						break;
					case FREQ_48:
						print_dbg_char_hex(0x48);
						break;
					case FREQ_88:
						print_dbg_char_hex(0x88);
						break;
					case FREQ_96:
						print_dbg_char_hex(0x96);
						break;
					case FREQ_176:
						print_dbg_char_hex(0x17);
						break;
					case FREQ_192:
						print_dbg_char_hex(0x19);
						break;
				}
            	print_dbg_char('\n');

*/
			}	// Done handling interrupt


			// Monitor silent or disconnected WM8805 input
			if (!WM_IS_SILENT()) {
				if ( (gpio_get_pin_value(WM8805_ZERO_PIN) == 1) || (wm8805_unlocked() ) ) {		// Is the WM8805 zero flag set, or is it in unlock?
					if (gpio_get_pin_value(WM8805_ZERO_PIN) == 1)
						wm8805_zerotimer += SILENCE_WM_ZERO;	// The poll intervals are crap, need thorough adjustment!
					else
						wm8805_zerotimer += SILENCE_WM_UNLINK;
				}
				else
					wm8805_zerotimer = SILENCE_WM_INIT;			// Not silent and in lock!
			}

			// Monitor loud WM8805 to qualify that it's actually sending music
			if (!WM_IS_LOUD()) {
				if (gpio_get_pin_value(WM8805_ZERO_PIN) == 0)
					wm8805_loudtimer += LOUD_WM_INC;
				else
					wm8805_loudtimer = LOUD_WM_INIT;
			}

    	} // else, !readkey

    	if (gotcmd == 0)									// Nothing recorded:
			vTaskDelay(120);								// Polling cycle gives 12ms to RTOS
    }

//  Tested ReportByte1 content with JRiver and VLC on Win7-32
//  ReportByte1 = 0b00000001; // Encode volup according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works!
//  ReportByte1 = 0b00000010; // Encode voldn according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works!
//  ReportByte1 = 0b00000100; // Encode PlayPause according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works!
//  ReportByte1 = 0b00001000; // Encode ScanNextTrack according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works! Holding key in VLC causes skips, not scanning forward within track
//  ReportByte1 = 0b00010000; // Encode ScanPrevTrack according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works like above! But VLC isn't all that graceful about skipping to the track before the first one..
//  ReportByte1 = 0b00100000; // Encode Stop according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works!
//  ReportByte1 = 0b01000000; // Encode FastForward to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works in JRiver, not VLC
//  ReportByte1 = 0b10000000; // Encode Rewind to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works in JRiver, not VLC

	#ifdef HID2LCD
		lcd_q_goto(0,0);
		lcd_q_putc('h');
		lcd_q_puth(ReportByte1);
		lcd_q_puth(ReportByte2);
	#endif


	// Send the HID report over USB

    if ( Is_usb_in_ready(EP_HID_TX) )
    {
       Usb_reset_endpoint_fifo_access(EP_HID_TX);
       Usb_write_endpoint_data(EP_HID_TX, 8, ReportByte0);
       Usb_write_endpoint_data(EP_HID_TX, 8, ReportByte1);
       Usb_write_endpoint_data(EP_HID_TX, 8, ReportByte2);
       Usb_ack_in_ready_send(EP_HID_TX);
       print_dbg_char_char('H');					// Confirm HID command forwarded to HOST
       print_dbg_char_char('\n');					// Confirm HID command forwarded to HOST
       #ifdef HID2LCD
         lcd_q_putc('H');
       #endif
       // usb_state = 'r'; // May we ignore usb_state for HID TX ??
    }
    else { // Failure
        print_dbg_char_char('-');					// NO HID command forwarded to HOST
        print_dbg_char_char('\n');					// NO HID command forwarded to HOST
        #ifdef HID2LCD
          lcd_q_putc('-');
        #endif
    }

    // BSB 20120711: Debugging HID end


#ifdef FREERTOS_USED
  }
#endif
}
#endif  // USB_DEVICE_FEATURE == ENABLED
