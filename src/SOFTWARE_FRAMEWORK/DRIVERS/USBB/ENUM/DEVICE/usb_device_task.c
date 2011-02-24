/* This source file is part of the ATMEL AVR-UC3-SoftwareFramework-1.7.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file ******************************************************************
 *
 * \brief Management of the USB device controller.
 *
 * The USB task checks the income of new requests from the USB host.
 * When a setup request occurs, this task launches the processing of this
 * setup contained in the usb_standard_request.c file.
 * Other class-specific requests are also processed in this file.
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
 */

//_____  I N C L U D E S ___________________________________________________

#include "conf_usb.h"


#if USB_DEVICE_FEATURE == ENABLED

#include "compiler.h"
#ifdef FREERTOS_USED
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "usb_drv.h"
#include "usb_task.h"
#include "usb_descriptors.h"
#include "usb_standard_request.h"
#include "usb_device_task.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

//!
//! Public : (Bool) usb_connected
//! usb_connected is set to TRUE when VBus has been detected
//! usb_connected is set to FALSE otherwise
//!
volatile Bool usb_connected;

#ifdef FREERTOS_USED
//! Handle to the USB Device task
xTaskHandle usb_device_tsk = NULL;
#endif


//_____ D E C L A R A T I O N S ____________________________________________

//!
//! @brief This function initializes the USB device controller.
//!
//! This function enables the USB controller and inits the USB interrupts.
//! The aim is to allow the USB connection detection in order to send
//! the appropriate USB event to the operating mode manager.
//!
void usb_device_task_init(void)
{
  usb_connected = FALSE;
  usb_configuration_nb = 0;
  //! @todo Implement this on the silicon version
  //Pll_start_auto();
  //Wait_pll_ready();
  Disable_global_interrupt();
  Usb_disable();
  (void)Is_usb_enabled();
  Enable_global_interrupt();
  Usb_disable_otg_pad();
  Usb_enable_otg_pad();
  Usb_enable();
  Usb_unfreeze_clock();
  (void)Is_usb_clock_frozen();
  Usb_ack_suspend();  // A suspend condition may be detected right after enabling the USB macro
  Usb_enable_vbus_interrupt();
  Enable_global_interrupt();

#ifdef FREERTOS_USED
  xTaskCreate(usb_device_task,
              configTSK_USB_DEV_NAME,
              configTSK_USB_DEV_STACK_SIZE,
              NULL,
              configTSK_USB_DEV_PRIORITY,
              &usb_device_tsk);
#endif  // FREERTOS_USED
}


//!
//! @brief This function starts the USB device controller.
//!
//! This function enables the USB controller and inits the USB interrupts.
//! The aim is to allow the USB connection detection in order to send
//! the appropriate USB event to the operating mode manager.
//! Start device function is executed once VBus connection has been detected
//! either by the VBus change interrupt or by the VBus high level.
//!
void usb_start_device(void)
{
  Usb_enable_suspend_interrupt();
  Usb_enable_reset_interrupt();

#if (USB_HIGH_SPEED_SUPPORT==DISABLED)
  Usb_force_full_speed_mode();
#else
  Usb_use_dual_speed_mode();
#endif
  
  usb_init_device();  // Configure the USB controller EP0
  Usb_attach();
  usb_connected = TRUE;
}


//!
//! @brief Entry point of the USB device mamagement
//!
//! This function is the entry point of the USB management. Each USB
//! event is checked here in order to launch the appropriate action.
//! If a Setup request occurs on the Default Control Endpoint,
//! the usb_process_request() function is call in the usb_standard_request.c file
//!
#ifdef FREERTOS_USED
void usb_device_task(void *pvParameters)
#else
void usb_device_task(void)
#endif
{
#ifdef FREERTOS_USED
  portTickType xLastWakeTime;

  xLastWakeTime = xTaskGetTickCount();
  while (TRUE)
  {
    vTaskDelayUntil(&xLastWakeTime, configTSK_USB_DEV_PERIOD);

#endif  // FREERTOS_USED
    if (!usb_connected && Is_usb_vbus_high())
    {
      usb_start_device();
      Usb_send_event(EVT_USB_POWERED);
      Usb_vbus_on_action();
    }

    if (Is_usb_event(EVT_USB_RESET))
    {
      Usb_ack_event(EVT_USB_RESET);
      Usb_reset_endpoint(EP_CONTROL);
      usb_configuration_nb = 0;
    }

    // Connection to the device enumeration process
    if (Is_usb_setup_received())
    {
      usb_process_request();
    }
#ifdef FREERTOS_USED
  }
#endif
}


#endif  // USB_DEVICE_FEATURE == ENABLED
