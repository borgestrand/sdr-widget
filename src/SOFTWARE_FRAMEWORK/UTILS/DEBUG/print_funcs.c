/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief Strings and integers print module for debug purposes.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with a USART module can be used.
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ******************************************************************************/

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
 * Modified by Alex Lee 20 Feb 2010
 * To enumerate as a USB composite device with 4 interfaces:
 * CDC
 * HID (generic HID interface, compatible with Jan Axelson's generichid.exe test programs
 * DG8SAQ (libusb API compatible interface for implementing DG8SAQ EP0 type of interface)
 * Audio (USB Audio Class V2)
 *
 * For sdr-widget and audio-widget, custom boards based on the AT32UC3A3256
 *
 * See http://code.google.com/p/sdr-widget/
 *
 */

#include "compiler.h"
#include "gpio.h"
#include "usart.h"
#include "print_funcs.h"
//#include "uart_usb_lib.h"

//! ASCII representation of hexadecimal digits.
static const char HEX_DIGITS[16] = "0123456789ABCDEF";

/////////////////////////////////////////////
// BSB 20110127-20120717 Added read functions
/////////////////////////////////////////////

char read_dbg_char(char echo, char checksum_mode)
{
	volatile static char dbg_checksum = 0; // should be uint8_t??
	char read_data; // should be uint8_t??

	// dbg_checksum is a crude checksum mechanism compatible by other debug code by BSB.

	if (checksum_mode==DBG_CHECKSUM_NORMAL)
	{
		// Redirection to the debug USART.
	  	read_data = usart_getchar(DBG_USART); // returns int
	  	if (echo==DBG_ECHO)
	  		usart_putchar(DBG_USART, read_data);
		dbg_checksum += read_data;	// Checksum function is addition...
		dbg_checksum &= 0xFF;		// ... of which we save 8 lsbs. Redundant code line?
		return read_data;
	}
	else if (checksum_mode==DBG_CHECKSUM_RESET)
		dbg_checksum = 0;
	// Last alternative, DBG_CHECKSUM_READOUT, not tested
	return dbg_checksum;
}

char read_dbg_char_hex(char echo)
{
	char temp;
	char hexbyte=0;
	char counter=2;	// We're receiving 2 nibbles at a time

	while (counter > 0) {
		counter --;										// Assume valid character
		temp = read_dbg_char(echo, DBG_CHECKSUM_NORMAL);	// Get character with local echo, no checksum reporting
		if ( (temp >= 0x30) && (temp <= 0x39) )		// 0x30 encodes '0', 0x39 encodes '9'
			hexbyte += temp - 0x30;
		else if ( (temp >= 0x41) && (temp <= 0x46) )	// 0x41 encodes 'A', 0x46 encodes 'F'
			hexbyte += temp - 0x41 + 0x0A;
		else if ( (temp >= 0x61) && (temp <= 0x66) )	// 0x61 encodes 'a', 0x66 encodes 'f'
			hexbyte += temp - 0x61 + 0x0A;
		else {
			counter ++;									// Disqualify non-hex character
			temp = 0;
		}
		if ( (counter == 1) && (temp != 0) )			// If we just got the 1st nibble and its valid,
			hexbyte <<= 4;								// Shift high nibble
	}
	return hexbyte;
}

void print_dbg_char_char(int c)
{
	// Redirection to the debug USART.
	usart_putchar(DBG_USART, c);
}

/////////////////////////////////////////////
// BSB 20110127-20120717 End of insertions
/////////////////////////////////////////////



void init_dbg_rs232(long pba_hz)
{
  init_dbg_rs232_ex(DBG_USART_BAUDRATE, pba_hz);
}


void init_dbg_rs232_ex(unsigned long baudrate, long pba_hz)
{
  static const gpio_map_t DBG_USART_GPIO_MAP =
  {
    {DBG_USART_RX_PIN, DBG_USART_RX_FUNCTION},
    {DBG_USART_TX_PIN, DBG_USART_TX_FUNCTION}
  };

  // Options for debug USART.
  usart_options_t dbg_usart_options =
  {
    .baudrate = baudrate,
    .charlength = 8,
    .paritytype = USART_NO_PARITY,
    .stopbits = USART_1_STOPBIT,
    .channelmode = USART_NORMAL_CHMODE
  };

  // Setup GPIO for debug USART.
  gpio_enable_module(DBG_USART_GPIO_MAP,
                     sizeof(DBG_USART_GPIO_MAP) / sizeof(DBG_USART_GPIO_MAP[0]));

  // Initialize it in RS232 mode.
  usart_init_rs232(DBG_USART, &dbg_usart_options, pba_hz);
}


void print_dbg(const char *str)
{
  // Redirection to the debug USART.
  print(DBG_USART, str);

}


void print_dbg_char(int c)
{
  // Redirection to the debug USART.
  print_char(DBG_USART, c);
}


void print_dbg_ulong(unsigned long n)
{
  // Redirection to the debug USART.
  print_ulong(DBG_USART, n);
}


void print_dbg_char_hex(unsigned char n)
{
  // Redirection to the debug USART.
  print_char_hex(DBG_USART, n);
}


void print_dbg_short_hex(unsigned short n)
{
  // Redirection to the debug USART.
  print_short_hex(DBG_USART, n);
}


void print_dbg_hex(unsigned long n)
{
  // Redirection to the debug USART.
  print_hex(DBG_USART, n);
}


void print(volatile avr32_usart_t *usart, const char *str)
{
  // Invoke the USART driver to transmit the input string with the given USART.
  usart_write_line(usart, str);
}


void print_char(volatile avr32_usart_t *usart, int c)
{
  char tmp[2];
  // Invoke the USART driver to transmit the input character with the given USART.
  tmp[0] = c;
  tmp[1] = 0;
  print(usart, tmp);
}


void print_ulong(volatile avr32_usart_t *usart, unsigned long n)
{
  char tmp[11];
  int i = sizeof(tmp) - 1;

  // Convert the given number to an ASCII decimal representation.
  tmp[i] = '\0';
  do
  {
    tmp[--i] = '0' + n % 10;
    n /= 10;
  } while (n);

  // Transmit the resulting string with the given USART.
  print(usart, tmp + i);
}


void print_char_hex(volatile avr32_usart_t *usart, unsigned char n)
{
  char tmp[3];
  int i;

  // Convert the given number to an ASCII hexadecimal representation.
  tmp[2] = '\0';
  for (i = 1; i >= 0; i--)
  {
    tmp[i] = HEX_DIGITS[n & 0xF];
    n >>= 4;
  }

  // Transmit the resulting string with the given USART.
  print(usart, tmp);
}


void print_short_hex(volatile avr32_usart_t *usart, unsigned short n)
{
  char tmp[5];
  int i;

  // Convert the given number to an ASCII hexadecimal representation.
  tmp[4] = '\0';
  for (i = 3; i >= 0; i--)
  {
    tmp[i] = HEX_DIGITS[n & 0xF];
    n >>= 4;
  }

  // Transmit the resulting string with the given USART.
  print(usart, tmp);
}


void print_hex(volatile avr32_usart_t *usart, unsigned long n)
{
  char tmp[9];
  int i;

  // Convert the given number to an ASCII hexadecimal representation.
  tmp[8] = '\0';
  for (i = 7; i >= 0; i--)
  {
    tmp[i] = HEX_DIGITS[n & 0xF];
    n >>= 4;
  }

  // Transmit the resulting string with the given USART.
  print(usart, tmp);
}
