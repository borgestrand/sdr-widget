/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief AT32UC3A3 EVK1104 board LEDs support package.
 *
 * This file contains definitions and services related to the LED features of
 * the EVK1104 board.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 AT32UC3B devices can be used.
 * - AppNote:
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
 * *
 * Additions and Modifications to ATMEL AVR32-SoftwareFramework-AT32UC3 are:
 *
 * Copyright (C) Alex Lee
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

#include <avr32/io.h>
#include "gpio.h"
#include "preprocessor.h"
#include "compiler.h"
#include "SDRwdgt.h"
#include "led.h"


//! Structure describing LED hardware connections.
typedef const struct
{
  struct
  {
    U32 PORT;     //!< LED GPIO port.
    U32 PIN_MASK; //!< Bit-mask of LED pin in GPIO port.
  } GPIO; //!< LED GPIO descriptor.
  struct
  {
    S32 CHANNEL;  //!< LED PWM channel (< 0 if N/A).
    S32 FUNCTION; //!< LED pin PWM function (< 0 if N/A).
  } PWM;  //!< LED PWM descriptor.
} tLED_DESCRIPTOR;


//! Hardware descriptors of all LEDs.
static tLED_DESCRIPTOR LED_DESCRIPTOR[LED_COUNT] =
{
#define INSERT_LED_DESCRIPTOR(LED_NO, unused)                 \
  {                                                           \
    {LED##LED_NO##_GPIO / 32, 1 << (LED##LED_NO##_GPIO % 32)} \
  },
  MREPEAT(LED_COUNT, INSERT_LED_DESCRIPTOR, ~)
#undef INSERT_LED_DESCRIPTOR
};


//! Saved state of all LEDs.
static volatile U32 LED_State = (1 << LED_COUNT) - 1;


U32 LED_Read_Display(void)
{
  return LED_State;
}


void LED_Display(U32 leds)
{
  // Use the LED descriptors to get the connections of a given LED to the MCU.
  tLED_DESCRIPTOR *led_descriptor;
  volatile avr32_gpio_port_t *led_gpio_port;

  // Make sure only existing LEDs are specified.
  leds &= (1 << LED_COUNT) - 1;

  // Update the saved state of all LEDs with the requested changes.
  LED_State = leds;

  // For all LEDs...
  for (led_descriptor = &LED_DESCRIPTOR[0];
       led_descriptor < LED_DESCRIPTOR + LED_COUNT;
       led_descriptor++)
  {
    // Set the LED to the requested state.
    led_gpio_port = &AVR32_GPIO.port[led_descriptor->GPIO.PORT];
    if (leds & 1)
    {
      led_gpio_port->ovrc  = led_descriptor->GPIO.PIN_MASK;
    }
    else
    {
      led_gpio_port->ovrs  = led_descriptor->GPIO.PIN_MASK;
    }
    led_gpio_port->oders = led_descriptor->GPIO.PIN_MASK;
    led_gpio_port->gpers = led_descriptor->GPIO.PIN_MASK;
    leds >>= 1;
  }
}


U32 LED_Read_Display_Mask(U32 mask)
{
  return Rd_bits(LED_State, mask);
}


void LED_Display_Mask(U32 mask, U32 leds)
{
  // Use the LED descriptors to get the connections of a given LED to the MCU.
  tLED_DESCRIPTOR *led_descriptor = &LED_DESCRIPTOR[0] - 1;
  volatile avr32_gpio_port_t *led_gpio_port;
  U8 led_shift;

  // Make sure only existing LEDs are specified.
  mask &= (1 << LED_COUNT) - 1;

  // Update the saved state of all LEDs with the requested changes.
  Wr_bits(LED_State, mask, leds);

  // While there are specified LEDs left to manage...
  while (mask)
  {
    // Select the next specified LED and set it to the requested state.
    led_shift = 1 + ctz(mask);
    led_descriptor += led_shift;
    led_gpio_port = &AVR32_GPIO.port[led_descriptor->GPIO.PORT];
    leds >>= led_shift - 1;
    if (leds & 1)
    {
      led_gpio_port->ovrc  = led_descriptor->GPIO.PIN_MASK;
    }
    else
    {
      led_gpio_port->ovrs  = led_descriptor->GPIO.PIN_MASK;
    }
    led_gpio_port->oders = led_descriptor->GPIO.PIN_MASK;
    led_gpio_port->gpers = led_descriptor->GPIO.PIN_MASK;
    leds >>= 1;
    mask >>= led_shift;
  }
}


Bool LED_Test(U32 leds)
{
  return Tst_bits(LED_State, leds);
}

// Actual GPIO manipulation
void LED_Off_GPIO(U32 leds)
{
  // Use the LED descriptors to get the connections of a given LED to the MCU.
  tLED_DESCRIPTOR *led_descriptor = &LED_DESCRIPTOR[0] - 1;
  volatile avr32_gpio_port_t *led_gpio_port;
  U8 led_shift;

  // Make sure only existing LEDs are specified.
  leds &= (1 << LED_COUNT) - 1;

  // Update the saved state of all LEDs with the requested changes.
  Clr_bits(LED_State, leds);

  // While there are specified LEDs left to manage...
  while (leds)
  {
    // Select the next specified LED and turn it off.
    led_shift = 1 + ctz(leds);
    led_descriptor += led_shift;
    led_gpio_port = &AVR32_GPIO.port[led_descriptor->GPIO.PORT];
    led_gpio_port->ovrc  = led_descriptor->GPIO.PIN_MASK;
    led_gpio_port->oders = led_descriptor->GPIO.PIN_MASK;
    led_gpio_port->gpers = led_descriptor->GPIO.PIN_MASK;
    leds >>= led_shift;
  }
}

// Actual GPIO manipulation
void LED_On_GPIO(U32 leds)
{
  // Use the LED descriptors to get the connections of a given LED to the MCU.
  tLED_DESCRIPTOR *led_descriptor = &LED_DESCRIPTOR[0] - 1;
  volatile avr32_gpio_port_t *led_gpio_port;
  U8 led_shift;

  // Make sure only existing LEDs are specified.
  leds &= (1 << LED_COUNT) - 1;

  // Update the saved state of all LEDs with the requested changes.
  Set_bits(LED_State, leds);

  // While there are specified LEDs left to manage...
  while (leds)
  {
    // Select the next specified LED and turn it on.
    led_shift = 1 + ctz(leds);
    led_descriptor += led_shift;
    led_gpio_port = &AVR32_GPIO.port[led_descriptor->GPIO.PORT];
    led_gpio_port->ovrs  = led_descriptor->GPIO.PIN_MASK;
    led_gpio_port->oders = led_descriptor->GPIO.PIN_MASK;
    led_gpio_port->gpers = led_descriptor->GPIO.PIN_MASK;
    leds >>= led_shift;
  }
}


//Function called by other code
void LED_Off(U32 leds) {
	#if (defined HW_GEN_SPRX)
		LED_Off_GPIO(leds);								// RXMODFIX LEDs on PCB are active high, LEDs at edge are active low
	#else
		gpio_enable_pin_pull_up(AVR32_PIN_PA04);		// Floating: Active high. GND: Active low HW_GEN_SPRX: stay away from PA04! usbmod and HW_GEN_FMADC brought out to test point

		if (gpio_get_pin_value(AVR32_PIN_PA04) == 1)	// Active high
			LED_Off_GPIO(leds);
		else											// Active low
			LED_On_GPIO(leds);

		gpio_disable_pin_pull_up(AVR32_PIN_PA04);		// Floating: Active high. GND: Active low
	#endif
}


//Function called by other code
void LED_On(U32 leds) {
	#if (defined HW_GEN_SPRX)
		LED_On_GPIO(leds);								// RXMODFIX LEDs on PCB are active high, LEDs at edge are active low
	#else
		gpio_enable_pin_pull_up(AVR32_PIN_PA04);		// Floating: Active high. GND: Active low HW_GEN_SPRX: stay away from PA04! usbmod and HW_GEN_FMADC brought out to test point

		if (gpio_get_pin_value(AVR32_PIN_PA04) == 1)	// Active high
			LED_On_GPIO(leds);
		else											// Active low
			LED_Off_GPIO(leds);

		gpio_disable_pin_pull_up(AVR32_PIN_PA04);		// Floating: Active high. GND: Active low
	#endif
}



void LED_Toggle(U32 leds)
{
  // Use the LED descriptors to get the connections of a given LED to the MCU.
  tLED_DESCRIPTOR *led_descriptor = &LED_DESCRIPTOR[0] - 1;
  volatile avr32_gpio_port_t *led_gpio_port;
  U8 led_shift;

  // Make sure only existing LEDs are specified.
  leds &= (1 << LED_COUNT) - 1;

  // Update the saved state of all LEDs with the requested changes.
  Tgl_bits(LED_State, leds);

  // While there are specified LEDs left to manage...
  while (leds)
  {
    // Select the next specified LED and toggle it.
    led_shift = 1 + ctz(leds);
    led_descriptor += led_shift;
    led_gpio_port = &AVR32_GPIO.port[led_descriptor->GPIO.PORT];
    led_gpio_port->ovrt  = led_descriptor->GPIO.PIN_MASK;
    led_gpio_port->oders = led_descriptor->GPIO.PIN_MASK;
    led_gpio_port->gpers = led_descriptor->GPIO.PIN_MASK;
    leds >>= led_shift;
  }
}


U32 LED_Read_Display_Field(U32 field)
{
  return Rd_bitfield(LED_State, field);
}


void LED_Display_Field(U32 field, U32 leds)
{
  // Move the bit-field to the appropriate position for the bit-mask.
  LED_Display_Mask(field, leds << ctz(field));
}

U8 LED_Get_Intensity(U32 led)
{
 // always return 0, UC3A3 don't have PWM to modulate led intensity.
 return 0;
}

void LED_Set_Intensity(U32 leds, U8 intensity)
{
  // Empty Function, UC3A3 don't have PWM to modulate led intensity.
}
