/*
 * BlinkyFlashy.c
 *
 *  Created on: 03-Sep-2010
 *      Author: alex
 */

/* This source file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief GPIO example application for AVR32 using the peripheral bus interface.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with GPIO.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 *****************************************************************************/

/*! \page License
 * Copyright (c) 2009 Atmel Corporation. All rights reserved.
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
 */

/*! \mainpage
 * \section intro Introduction
 * This is the documentation for the data structures, functions, variables,
 * defines, enums, and typedefs for the GPIO driver.
 *
 * The General Purpose Input/Output manages the I/O pins of the microcontroller. Each I/O line
 * may be dedicated as a general-purpose I/O or be assigned to a function of an embedded peripheral.
 * This assures effective optimization of the pins of a product.
 *
 * The given example covers various uses of the GPIO controller and demonstrates
 * different GPIO functionalities using the peripheral bus interface. It uses a
 * LED and a button.
 *
 * This interface operates with lower clock frequencies (fPB <= fCPU), and its
 * timing is not deterministic since it needs to access a shared bus which may
 * be heavily loaded.
 *
 * \section files Main Files
 * - gpio.c: GPIO driver;
 * - gpio.h: GPIO driver header file;
 * - gpio_peripheral_bus_example.c: GPIO example application using the peripheral bus.
 *
 * \section compinfo Compilation Info
 * This software was written for the GNU GCC for AVR32 and IAR Systems compiler
 * for AVR32. Other compilers may or may not work.
 *
 * \section deviceinfo Device Info
 * All AVR32 devices with a GPIO module can be used. This example has been tested
 * with the following setup:
 *   - EVK1100, EVK1101, EVK1103, EVK1104 or EVK1105 evaluation kits
 *   - STK600+RCUC3L0 routing card: connect STK600.PORTA.PA4 to STK600.LEDS.LED0,
 *     STK600.PORTA.PA5 to STK600.LEDS.LED1, STK600.PORTD.PD2 to STK600.SWITCHES.SW0.
 *     Press and release SW0 to turn LED1 on/off. LED0 is automatically toggled by
 *     the application
 *   - AT32UC3L-EK: the A LED toggles forever and use the WAKE button to toggle the B LED.
 *
 *
 * \section setupinfo Setup Information
 * CPU speed: <i> Internal RC oscillator (about 115200 Hz) </i>.
 *
 * \section contactinfo Contact Information
 * For further information, visit
 * <A href="http://www.atmel.com/products/AVR32/">Atmel AVR32</A>.\n
 * Support and FAQ: http://support.atmel.no/
 */


#include "compiler.h"
#include "gpio.h"
#include "board.h"
//
#include "conf_isp.h"

/*! \name Pin Configuration
 */
//! @{
#define GPIO_PIN_EXAMPLE_1  LED1_GPIO
#define GPIO_PIN_EXAMPLE_2  LED0_GPIO

#define TWI_SCL  AVR32_PIN_PA14
#define TWI_SDA  AVR32_PIN_PA15

//#define GPIO_CW_KEY_1        AVR32_PIN_PB9
#define GPIO_CW_KEY_1        AVR32_PIN_PX00
#define GPIO_CW_KEY_2        AVR32_PIN_PX01
#define GPIO_PTT_INPUT       AVR32_PIN_PX03

#define PTT_1				 AVR32_PIN_PX45
#define PTT_2				 AVR32_PIN_PX42
#define PTT_3				 AVR32_PIN_PX22

#if BOARD == EVK1104
  #define GPIO_PIN_EXAMPLE_3  GPIO_PUSH_BUTTON_SW2
#elif BOARD == SDRwdgt || BOARD == SDRwdgtLite
  #define GPIO_PIN_EXAMPLE_3  GPIO_PUSH_BUTTON_SW2
#endif

#if !defined(GPIO_PIN_EXAMPLE_1) || \
    !defined(GPIO_PIN_EXAMPLE_2) || \
    !defined(GPIO_PIN_EXAMPLE_3)
  #error The pin configuration to use in this example is missing.
#endif
//! @}



/*! \brief This is an example of how to access the gpio.c driver to set, clear, toggle... the pin GPIO_PIN_EXAMPLE.
 */
int main(void)
{

  U32 i;

  gpio_enable_pin_glitch_filter(GPIO_PIN_EXAMPLE_3);
  gpio_enable_pin_pull_up(GPIO_CW_KEY_1);
  gpio_enable_pin_pull_up(GPIO_CW_KEY_2);
  gpio_enable_pin_pull_up(GPIO_PTT_INPUT);

  while (1)
  {

      // Note that it is also possible to use the GPIO toggle feature.
      gpio_tgl_gpio_pin(GPIO_PIN_EXAMPLE_1);


    // Poll push button value.
    for (i = 0; i < 200; i++)
    {
      if (gpio_get_pin_value(GPIO_PIN_EXAMPLE_3) == 0 || \
    		  gpio_get_pin_value(GPIO_CW_KEY_1) == 0 || \
    		  gpio_get_pin_value(GPIO_CW_KEY_2) == 0 || \
    		  gpio_get_pin_value(GPIO_PTT_INPUT) == 0
      ){
        gpio_clr_gpio_pin(GPIO_PIN_EXAMPLE_2);
		gpio_clr_gpio_pin(TWI_SCL);
		gpio_clr_gpio_pin(TWI_SDA);
		gpio_set_gpio_pin(PTT_1);
		gpio_set_gpio_pin(PTT_2);
		gpio_set_gpio_pin(PTT_3);
      }
      else {
        gpio_set_gpio_pin(GPIO_PIN_EXAMPLE_2);
		gpio_set_gpio_pin(TWI_SCL);
		gpio_set_gpio_pin(TWI_SDA);
		gpio_clr_gpio_pin(PTT_1);
		gpio_clr_gpio_pin(PTT_2);
		gpio_clr_gpio_pin(PTT_3);
        }
    }

  }
}
