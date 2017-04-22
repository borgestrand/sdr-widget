/* This source file is part of the ATMEL AVR-UC3-SoftwareFramework-1.7.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Management of the USB task either device/host or both.
 *
 * The USB task selects the correct USB task (USB device task or USB host
 * task) to be executed depending on the current mode available.
 *
 * According to the values of USB_DEVICE_FEATURE and USB_HOST_FEATURE
 * (located in the conf_usb.h file), the USB task can be configured to
 * support USB device mode or USB host mode or both for a dual-role device
 * application.
 *
 * This module also contains the general USB interrupt subroutine. This
 * subroutine is used to detect asynchronous USB events.
 *
 * Note:
 *   - The USB task belongs to main; the USB device and host tasks do not.
 *     They are called from the general USB task.
 *   - See the conf_usb.h file for more details about the configuration of
 *     this module.
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
 * Modified by Borge Strand-Bergesen (Henry Audio) 23 Aug 2015
 * To tolerate external power and USB cable plug in and out (and other varieties)
 * For Audio Widget, custom boards based on the AT32UC3A3256
 *
 * See http://code.google.com/p/sdr-widget/
 *
 * Additions and Modifications to ATMEL AVR32-SoftwareFramework-AT32UC3 are:
 *
 * Copyright (C) Borge Strand-Bergesen
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

//_____  I N C L U D E S ___________________________________________________

#include "compiler.h"
#include "intc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "conf_usb.h"
#include "usb_drv.h"
#include "usb_task.h"
#include "gpio.h"


#include "usb_descriptors.h"
#include "usb_device_task.h"

#if UC3C
#include "pm_uc3c.h"
#else
#include "pm.h"
#endif


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

//!
//! Public: U16 g_usb_event
//! usb_connected is used to store USB events detected upon
//! USB general interrupt subroutine
//! Its value is managed by the following macros (See \ref usb_task.h file)
//! Usb_send_event(x)
//! Usb_ack_event(x)
//! Is_usb_event(x)
//! Usb_clear_all_event()
volatile U16 g_usb_event = 0;
#if ((USB_HOST_FEATURE == ENABLED) || (USB_DEVICE_FEATURE == ENABLED)) && (USB_HIGH_SPEED_SUPPORT==ENABLED)
// static U8 private_sof_counter_HS = 0;  // Full speed SOF = 1ms , High speed µSOF = 125µs
#endif


//!
//! Public: Bool usb_connected
//! usb_connected is set to TRUE when VBus has been detected
//! usb_connected is set to FALSE otherwise
//! Used with USB_DEVICE_FEATURE == ENABLED only
extern volatile Bool usb_connected;

//! Handle to the USB Device task
extern xTaskHandle usb_device_tsk;

// BSB 20150823: Removing a bunch of #if USB_HOST_FEATURE == ENABLED for improved readability in Audio Widget project.

//! Handle to the USB task semaphore
static xSemaphoreHandle usb_tsk_semphr = NULL;


//_____ D E C L A R A T I O N S ____________________________________________


#if (defined __GNUC__)
	__attribute__((__noinline__))
#endif

static portBASE_TYPE usb_general_interrupt_non_naked(void);


//! @brief USB interrupt routine
//!
//! When FreeRTOS is used, the USB interrupt routine may trigger task switches,
//! so it must use special OS prologue and epilogue. This function must be naked
//! in order to have no stack frame. usb_general_interrupt_non_naked is
//! therefore used for the required stack frame of the interrupt routine.
#if (defined __GNUC__)
	__attribute__((__naked__))
#elif __ICCAVR32__
	#pragma shadow_registers = full
#endif

static void usb_general_interrupt(void)
{
  portENTER_SWITCHING_ISR();
  usb_general_interrupt_non_naked();
  portEXIT_SWITCHING_ISR();
}


//! @brief This function initializes the USB process.
//!
//! Depending on the mode supported (HOST/DEVICE/DUAL_ROLE) the function
//! calls the coresponding USB mode initialization function
void usb_task_init(void)
{
  // Create the semaphore
  vSemaphoreCreateBinary(usb_tsk_semphr);

  xTaskCreate(usb_task,
              configTSK_USB_NAME,
              configTSK_USB_STACK_SIZE,
              NULL,
              configTSK_USB_PRIORITY,
              NULL);
}


void usb_task(void *pvParameters)
{
  // Register the USB interrupt handler to the interrupt controller and enable
  // the USB interrupt.
//  print_dbg_char('V');

  Disable_global_interrupt();

  INTC_register_interrupt((__int_handler)&usb_general_interrupt, AVR32_USBB_IRQ, USB_INT_LEVEL);
//  print_dbg_char('W'); // Very strange behaviour! The timing of print_dbg_char here and '!' below is critical!!

// Try to move 'W' out of the interrupt lock-out, then implement with some delay. Check if freertos delay will work or hang. 
// Do we know the scheduler has started up properly when this function is called? Could the hang point be while waiting for the semaphore?
// With external power applied, try all 4 combinations of:
// Startup with USB signal present / not present
// VBUS logical level from external / usb cable
// All four must lead to clean boot and live indication in taskMoboCtrl


  Enable_global_interrupt();

//  print_dbg_char('X');

  while (TRUE)
  {
    // Wait for the semaphore
    while (!xSemaphoreTake(usb_tsk_semphr, portMAX_DELAY));


// print_dbg_char('Y');

// ---- DEVICE-ONLY USB MODE ---------------------------------------------------
  if (usb_device_tsk) vTaskDelete(usb_device_tsk), usb_device_tsk = NULL;

// print_dbg_char('Z');

  Usb_force_device_mode();

// print_dbg_char('v');

  usb_device_task_init();

//  print_dbg_char('w'); // UP Runs to here with both pluged and unplugged mode

  }
}



//! @brief USB interrupt routine
//!
//! This function is called each time a USB interrupt occurs.
//! The following USB DEVICE events are taken in charge:
//! - VBus On / Off
//! - Start-of-Frame
//! - Suspend
//! - Wake-Up
//! - Resume
//! - Reset
//!
//! The following USB HOST events are taken in charge:
//! - Device connection
//! - Device Disconnection
//! - Start-of-Frame
//! - ID pin change
//! - SOF (or Keep alive in low-speed) sent
//! - Wake-up on USB line detected
//! - Pipe events
//!
//! For each event, the user can launch an action by completing the associated
//! \#define (see the conf_usb.h file to add actions on events).
//!
//! Note: Only interrupt events that are enabled are processed.
//!
//! Warning: If device and host tasks are not tasks in an RTOS, rough events
//! like ID transition, VBus transition, device disconnection, etc. that need to
//! kill then restart these tasks may lead to an undefined state if they occur
//! just before something is activated in the USB macro (e.g. pipe/endpoint
//! transfer...).
//!
//! @return Nothing in the standalone configuration; a boolean indicating
//!         whether a task switch is required in the FreeRTOS configuration

#if (defined __GNUC__)
__attribute__((__noinline__))
#elif (defined __ICCAVR32__)
#pragma optimize = no_inline
#endif

static portBASE_TYPE usb_general_interrupt_non_naked(void) {
	portBASE_TYPE task_woken = pdFALSE;

// BSB debug 20150823
// print_dbg_char('!');


// ---------- DEVICE events management -----------------------------------------
    // VBus state detection
    if (Is_usb_vbus_transition() && Is_usb_vbus_interrupt_enabled())
    {
      Usb_ack_vbus_transition();
      if (Is_usb_vbus_high())
      {
        usb_start_device();
        Usb_send_event(EVT_USB_POWERED);
        Usb_vbus_on_action();
      }
      else
      {
        Usb_unfreeze_clock();
        Usb_detach();
        usb_connected = FALSE;
        usb_configuration_nb = 0;
        Usb_send_event(EVT_USB_UNPOWERED);
        Usb_vbus_off_action();
        // Release the semaphore in order to start a new device/host task
        taskENTER_CRITICAL();
        xSemaphoreGiveFromISR(usb_tsk_semphr, &task_woken);
        taskEXIT_CRITICAL();
      }
    }
    // Device Start-of-Frame received
    if (Is_usb_sof() && Is_usb_sof_interrupt_enabled())
    {
      Usb_ack_sof();
      Usb_sof_action();
    }
    // Device Suspend event (no more USB activity detected)
    if (Is_usb_suspend() && Is_usb_suspend_interrupt_enabled())
    {
      Usb_ack_suspend();
      Usb_enable_wake_up_interrupt();
      (void)Is_usb_wake_up_interrupt_enabled();
      Usb_freeze_clock();
      Usb_send_event(EVT_USB_SUSPEND);
      Usb_suspend_action();
    }
    // Wake-up event (USB activity detected): Used to resume
    if (Is_usb_wake_up() && Is_usb_wake_up_interrupt_enabled())
    {
      Usb_unfreeze_clock();
      (void)Is_usb_clock_frozen();
      Usb_ack_wake_up();
      Usb_disable_wake_up_interrupt();
      Usb_wake_up_action();
      Usb_send_event(EVT_USB_WAKE_UP);
    }
    // Resume state bus detection
    if (Is_usb_resume() && Is_usb_resume_interrupt_enabled())
    {
      Usb_disable_wake_up_interrupt();
      Usb_ack_resume();
      Usb_disable_resume_interrupt();
      Usb_resume_action();
      Usb_send_event(EVT_USB_RESUME);
    }
    // USB bus reset detection
    if (Is_usb_reset() && Is_usb_reset_interrupt_enabled())
    {
      Usb_ack_reset();
      usb_init_device();
      Usb_reset_action();
      Usb_send_event(EVT_USB_RESET);
    }

  return task_woken;
}


void usb_suspend_action(void)
{
   print_dbg_char('S');
   volatile avr32_pm_t *pm = &AVR32_PM;
   pm->AWEN.usb_waken = 1;
   SLEEP(AVR32_PM_SMODE_STATIC);
   pm->AWEN.usb_waken = 0;
}



