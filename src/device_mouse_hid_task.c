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
#include "device_audio_task.h"
#include "Mobo_config.h"
#include "features.h"
#include "pdca.h" // For ADC channel config tests
#include "taskAK5394A.h"

#ifdef USB_METALLIC_NOISE_SIM
#include "device_audio_task.h"	// To modify FB_rate_nominal
#endif

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
#if LCD_DISPLAY
	#define HID2LCD					// Use LCD to debug incoming HID commands from uart
#endif

#ifdef HID2LCD					// Needed here? Including these lines seems to break functionality
//	lcd_q_init();
//	lcd_q_clear();
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
  U8 ReportByte1 = 0;			// 1st variable byte of HID report
  U8 ReportByte2 = 0; 			// 2nd variable byte of HID report
  U8 ReportByte1_prev = 0;		// Previous ReportByte1
  char a = 0;					// ASCII character as part of HID protocol over uart
  char gotcmd = 0;				// Initially, no user command was recorded


#ifdef FREERTOS_USED
  portTickType xLastWakeTime;

  xLastWakeTime = xTaskGetTickCount();
  while (TRUE)
  {
    vTaskDelayUntil(&xLastWakeTime, configTSK_USB_DHID_MOUSE_PERIOD);

    // First, check the device enumeration state BSB 20160504 Let's drop that for now...
//    if (!Is_device_enumerated()) continue;
#else
    // First, check the device enumeration state
//    if (!Is_device_enumerated()) return;
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

            // If you need the UART for something other than HID, this is where you interpret it!


#ifdef USB_METALLIC_NOISE_SIM
            else if (a == 'u') {
            	FB_rate_nominal += 64;
            	FB_rate_nominal |= 1; // Use LSB as mask for FB rate being messed up
            }
            else if (a == 'd') {
            	FB_rate_nominal -= 64;
            	FB_rate_nominal |= 1; // Use LSB as mask for FB rate being messed up
            }
            else if (a == 'x') {
            	FB_rate_nominal = FB_rate_initial; // Use LSB as mask for FB rate being messed up
            }
#endif


#ifdef HW_GEN_DIN20
            else if (a == '0') {							// Digit 0
        		usb_ch = USB_CH_NONE;
            	mobo_usb_select(usb_ch);
            }
            else if (a == 'A') {							// Uppercase A
            	usb_ch = USB_CH_A;
            	mobo_usb_select(usb_ch);
            }
            else if (a == 'B') {							// Uppercase B
            	usb_ch = USB_CH_B;
            	mobo_usb_select(usb_ch);
            }
            else if (a == 'D') {							// Uppercase D
            	if (mobo_usb_detect() == USB_CH_A)
            		print_dbg_char('A');
            	else
            		print_dbg_char('B');
            }

            else if (a == 'Y') {							// Uppercase Y
            	mobo_i2s_enable(MOBO_I2S_ENABLE);
            }
            else if (a == 'y') {							// Lowercase y
            	mobo_i2s_enable(MOBO_I2S_DISABLE);
            }

            else if (a == 'F') {							// Uppercase F
            	mobo_km(MOBO_HP_KM_ENABLE);
            }

            else if (a == 'f') {							// Lowercase f
            	mobo_km(MOBO_HP_KM_DISABLE);
            }
#endif

#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)
            // Start messing with ADC!

            else if (a == 'a') {							// Lowercase a
                static const pdca_channel_options_t PDCA_OPTIONS_DBG = {
                	.addr = (void *)audio_buffer_0,         // memory address
                	.pid = AVR32_PDCA_PID_SSC_RX,           // select peripheral
                	.size = ADC_BUFFER_SIZE,              // transfer counter
                	.r_addr = NULL,                         // next memory address
                	.r_size = 0,                            // next transfer counter
                	.transfer_size = PDCA_TRANSFER_SIZE_WORD  // select size of the transfer - 32 bits
                };



            	pdca_init_channel(PDCA_CHANNEL_SSC_RX, &PDCA_OPTIONS_DBG); // init PDCA channel with options.
        		pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_SSC_RX);
				pdca_enable(PDCA_CHANNEL_SSC_RX);


        		print_dbg_char('a');

        		// This code was brought into taskAK5394A.c (init) and wm8805(enable)
            }

            // Select MCU's outgoing I2S bus
            else if (a == 'b') {							// Lowercase b
            }

            // Select RXs's outgoing I2S bus
            else if (a == 'c') {							// Lowercase b
            	gpio_set_gpio_pin(AVR32_PIN_PX44); 			// SEL_USBN_RXP = 1
       			gpio_clr_gpio_pin(AVR32_PIN_PX58); 			// 44.1 control
       			gpio_clr_gpio_pin(AVR32_PIN_PX45); 			// 48 control
            }

            // LED debug
            else if (a == 'L') {							// Uppercase L
            	// 3 hex characters to LEDs. Punched as 6 digits. 0x00-0x07 are valid.
            	// Left-to-right on device front: fled2, fled1, fled0, NO! it's the other way around?! Why??
            	// RED			1
				// GREEN		2
				// YELLOW		3
				// BLUE			4
				// PURPLE		5
				// CYAN			6
				// WHITE		7
				// DARK			0
            	mobo_led(read_dbg_char_hex(DBG_ECHO, RTOS_WAIT), read_dbg_char_hex(DBG_ECHO, RTOS_WAIT), read_dbg_char_hex(DBG_ECHO, RTOS_WAIT));
            }
#endif

            else if (a == 'v') {
            	static S16 temp = VOL_MIN;
            	S16 temp2;

            	usb_volume_flash(CH_LEFT, temp, VOL_WRITE);
            	temp2 = usb_volume_flash(CH_LEFT, 0, VOL_READ);
            	usb_volume_flash(CH_RIGHT, temp, VOL_WRITE);
            	temp2 = usb_volume_flash(CH_RIGHT, 0, VOL_READ);

            	print_dbg_char_hex(((temp2 >> 8) & 0xff));
            	print_dbg_char_hex(((temp2 >> 0) & 0xff));

            	temp ++;
            }



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

/* Moved to taskMoboCtrl.c
#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20)
			wm8805_poll();									// Handle WM8805's various hardware needs
#endif
*/

    	} // else, !readkey

    	if (gotcmd == 0)									// Nothing recorded:
			vTaskDelay(120);								// Polling cycle gives 12ms to RTOS. WM8805 needs that, HID doesn't
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
       print_dbg_char('H');					// Confirm HID command forwarded to HOST
       print_dbg_char('\n');					// Confirm HID command forwarded to HOST
       #ifdef HID2LCD
         lcd_q_putc('H');
       #endif
       // usb_state = 'r'; // May we ignore usb_state for HID TX ??
    }
    else { // Failure
        print_dbg_char('-');					// NO HID command forwarded to HOST
        print_dbg_char('\n');					// NO HID command forwarded to HOST
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
