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


////// For old debug commands see device_mouse_hid_task.c_legacy_commands /////

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

#include "ssc_i2s.h" // For I2S tests




//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________



//_____ D E C L A R A T I O N S ____________________________________________

// static U8 usb_state = 'r'; // BSB 20120718 unused variable, sane?
// static U8 ep_hid_rx; // BSB 20120718 unused variable, sane?
   static U8 ep_hid_tx;
   
   
static uint8_t I2C_device_address = 0;   // RXMODFIX I2C debug
   
//!
//! @brief This function initializes the hardware/software resources
//! required for device HID task.
//!
// void device_mouse_hid_task_init(U8 ep_rx, U8 ep_tx)
void device_mouse_hid_task_init(U8 ep_tx) {

#if BOARD == EVK1101
	// Initialize accelerometer driver
	acc_init();
#endif
//	ep_hid_rx = ep_rx; // BSB 20120718 unused variable, sane?
	ep_hid_tx = ep_tx;
#ifndef FREERTOS_USED 
// Removed reference to USB_HOST_FEATURE
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
	print_cpu_char(CPU_CHAR_BOOT);		// Tell CPU (when present) that CPU is booting up
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
  uint8_t temp, temp2;			// Temporary debug data
  uint32_t temp32;				// Temporary debug data

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



#ifdef HW_GEN_FMADC


			// Set preamp gain
            else if (a == 'g') {									// Lowercase g
	            temp = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);		// Channel 1 or 2, two hex nibbles
	            temp2 = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);		// Gain 0, 1, 2 or 3, two hex nibbles
				temp = mobo_fmadc_gain(temp, temp2);
				print_dbg_char('g');
				print_dbg_char_hex(temp);
				print_dbg_char('\n');
			}
	

			// I2C device address
			else if (a == 'r') {			// Static debug device address
				I2C_device_address = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);
				print_dbg_char('r');
				print_dbg_char('\n');
			}

			
			// I2C read
			else if (a == 'w') {								// Lowercase w - read (silly!)
				temp = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);	// Fetch local address
				if (mobo_i2c_read (&temp, I2C_device_address, temp) > 0) {
					print_dbg_char_hex(temp);
					print_dbg_char('+');
				}
				else {
					print_dbg_char('-');
				}
				print_dbg_char('\n');
			}


			// I2C write
			else if (a == 'W') {								// Uppercase W - write
				temp = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);	// Fetch local address
				temp2 = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);	// Fetch data to write
				if (mobo_i2c_write (I2C_device_address, temp, temp2) > 0) {
					print_dbg_char('+');
				}
				else  {
					print_dbg_char('-');
				}				
				print_dbg_char('\n');
			}
			            

			// Detect sample rate of I2S in
            else if (a == 's') {							// Lowercase s
				temp32 = mobo_srd();
				print_dbg_char_hex(temp32);
				print_dbg_char_hex(temp32 >> 8);
				print_dbg_char_hex(temp32 >> 16);
				print_dbg_char_hex(temp32 >> 24);
			}
#endif

#ifdef HW_GEN_RXMOD

// Also available in reduced debug system
            else if (a == MCU_CHAR_ALIVE) {					// Uppercase 'L' - live detect
	            print_cpu_char(CPU_CHAR_ALIVE);				// Answer with H for heartbeat
            }
			
			// Detect sample rate of I2S in
			else if (a == MCU_CHAR_SPRATE) {				// Lowercase 's'
				temp32 = mobo_srd();
				print_cpu_char_hex(temp32 >> 24);			// MSB in 32-bit fixed-point number
				print_cpu_char_hex(temp32 >> 16);
				print_cpu_char_hex(temp32 >> 8);
				print_cpu_char_hex(temp32);					// LSB in 32-bit fixed-point number
				print_cpu_char('\n');
			}

			// Debug spdif packet through timer/counter
			else if (a == 'q') {							// Lowercase 'q'
				print_dbg_char('\n');
				print_dbg_hex(min_last_written_ADC_pos);
				print_dbg_char('\n');
				print_dbg_hex(max_last_written_ADC_pos);
				print_dbg_char('\n');
			}

			// Detect sample rate of I2S in
			else if (a == MCU_CHAR_FBRATE) {				// Lowercase 'f'
				print_cpu_char_hex(FB_rate >> 24);			// MSB in 32-bit fixed-point number - not transmitted in UAC2 / Full Speed
				print_cpu_char_hex(FB_rate >> 16);			// MSB in 24-bit fixed-point number. Sample rate in kHz sent in 10.14 format
				print_cpu_char_hex(FB_rate >> 8);
				print_cpu_char_hex(FB_rate);				// LSB in 24-bit or 32-bit fixed-point number
				print_cpu_char('\n');
			}

			// Increase feedback rate 
			else if (a == MCU_CHAR_RATEUP) {				// Uppercase 'U'
				FB_rate += 64;								// Replicated FB_RATE_DELTA from Uac2_device_audio_task.c
			}

			// Increase feedback rate
			else if (a == MCU_CHAR_RATEDOWN) {				// Lowercase 'u'
				FB_rate -= 64;								// Replicated FB_RATE_DELTA from Uac2_device_audio_task.c
			}

// Normal debug system
            else if (a == '0') {							// Digit 0
	            // usb_ch = USB_CH_NONE;
				usb_ch = USB_CH_DEACTIVATE;					// Debugging disconnected USB cable
	            mobo_usb_select(usb_ch);
            }
            else if (a == 'C') {							// Uppercase C
	            usb_ch = USB_CH_C;
	            mobo_usb_select(usb_ch);
            }
            else if (a == 'B') {							// Uppercase B
	            usb_ch = USB_CH_B;
	            mobo_usb_select(usb_ch);
            }
            else if (a == 'D') {							// Uppercase D
	            if (mobo_usb_detect() == USB_CH_C)
					print_dbg_char('C');
	            else
					print_dbg_char('B');
            }

            else if (a == 'Y') {							// Uppercase Y
	            mobo_i2s_enable(MOBO_I2S_ENABLE);
            }

            else if (a == 'y') {							// Lowercase y
	            mobo_i2s_enable(MOBO_I2S_DISABLE);
            }

            else if (a == 'R') {							// Uppercase R
	            gpio_clr_gpio_pin(AVR32_PIN_PA25); 			// RESET_N / NSRST = 0
            }


/* Changing filters. TI says:
Hello, This is the information you needed: The interpolation filter can be changed with just 3 steps.

Enter standby mode 
Change the filter (W2b07)
Exit standby mode.
Kind Regards,

Arash
*/
            
            else if (a == 'r') {			// Static debug device address
				I2C_device_address = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);
			}

            else if (a == 'k') {			// PCM5142 filter selection. Valid: 01, 02, 03, 07
				temp = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);
				pcm5142_filter(temp);
            }


			// I2C read
			else if (a == 'w') {								// Lowercase w - read (silly!)
				temp = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);	// Fetch local address
				if (mobo_i2c_read (&temp, I2C_device_address, temp) > 0) {
					print_dbg_char_hex(temp);
					print_dbg_char('+');
				}
				else {
					print_dbg_char('-');
				}
				print_dbg_char('\n');
			}


			// I2C write
			else if (a == 'W') {								// Uppercase W - write
				temp = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);	// Fetch local address
				temp2 = read_dbg_char_hex(DBG_ECHO, RTOS_WAIT);	// Fetch data to write
				if (mobo_i2c_write (I2C_device_address, temp, temp2) > 0) {
					print_dbg_char('+');
				}
				else  {
					print_dbg_char('-');
				}
				print_dbg_char('\n');
			}
			
			
            // LED debug
            else if (a == 'L') {							// Uppercase L
	            // 1 hex characters to LED. 0x00-0x07 are valid.
	            // RED			1
	            // GREEN		2
	            // YELLOW		3
	            // BLUE			4
	            // PURPLE		5
	            // CYAN			6
	            // WHITE		7
	            // DARK			0
	            mobo_led(read_dbg_char_hex(DBG_ECHO, RTOS_WAIT));
            }

            // Check source and rate, output to LED and terminal
            else if (a == 'm') {
	            print_dbg_char_hex(input_select);			// Is source known?
	            print_dbg_char_hex( (uint8_t)(current_freq.frequency/1000) );			// Is rate known? 
	            mobo_led_select(current_freq.frequency, input_select);
            }
			
			
			// High-level I2S & MCLK MUX control
            else if (a == 'a') {							// Lowercase a
	            mobo_xo_select(spdif_rx_status.frequency, input_select);
            }

            // Select MCU's outgoing I2S bus
            else if (a == 'b') {							// Lowercase b
	            mobo_xo_select(spdif_rx_status.frequency, input_select);
            }
			


#endif // HW_GEN_RXMOD


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

    	} // else, !readkey

    	if (gotcmd == 0)									// Nothing recorded:
			vTaskDelay(120);								// Polling cycle gives 12ms to RTOS. WM8804 needs that, HID doesn't
    } //while (gotcmd == 0)


//  Tested ReportByte1 content with JRiver and VLC on Win7-32
//  ReportByte1 = 0b00000001; // Encode volup according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works!
//  ReportByte1 = 0b00000010; // Encode voldn according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works!
//  ReportByte1 = 0b00000100; // Encode PlayPause according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works!
//  ReportByte1 = 0b00001000; // Encode ScanNextTrack according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works! Holding key in VLC causes skips, not scanning forward within track
//  ReportByte1 = 0b00010000; // Encode ScanPrevTrack according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works like above! But VLC isn't all that graceful about skipping to the track before the first one..
//  ReportByte1 = 0b00100000; // Encode Stop according to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works!
//  ReportByte1 = 0b01000000; // Encode FastForward to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works in JRiver, not VLC
//  ReportByte1 = 0b10000000; // Encode Rewind to usb_hid_report_descriptor[USB_HID_REPORT_DESC] works in JRiver, not VLC

// Send the HID report over USB
#ifdef FEATURE_HID
    if ( Is_usb_in_ready(EP_HID_TX) )
    {
       Usb_reset_endpoint_fifo_access(EP_HID_TX);
       Usb_write_endpoint_data(EP_HID_TX, 8, ReportByte0); // HID report number, hard-coded to 0x01
       Usb_write_endpoint_data(EP_HID_TX, 8, ReportByte1);
       Usb_write_endpoint_data(EP_HID_TX, 8, ReportByte2);
       Usb_ack_in_ready_send(EP_HID_TX);
       print_dbg_char('H');					// Confirm HID command forwarded to HOST
       print_dbg_char('\n');					// Confirm HID command forwarded to HOST
       // usb_state = 'r'; // May we ignore usb_state for HID TX ??
    }
    else { // Failure
#else
    if (1) {
#endif
        print_dbg_char('-');					// NO HID command forwarded to HOST
        print_dbg_char('\n');					// NO HID command forwarded to HOST
    }

    // BSB 20120711: Debugging HID end


#ifdef FREERTOS_USED
  }  //   while (TRUE)
#endif
}
#endif  // USB_DEVICE_FEATURE == ENABLED
