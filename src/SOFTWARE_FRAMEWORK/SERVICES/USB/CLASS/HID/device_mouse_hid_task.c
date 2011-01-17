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
#if LCD_DISPLAY				// Multi-line LCD display
#include "taskLCD.h"
#endif
#include "taskEXERCISE.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________



//_____ D E C L A R A T I O N S ____________________________________________

static U8 usb_state = 'r';

//!
//! @brief This function initializes the hardware/software resources
//! required for device HID task.
//!
void device_mouse_hid_task_init(void)
{

#if BOARD == EVK1101
  // Initialize accelerometer driver
  acc_init();
#endif

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
	U8 data_length;

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

       switch (usb_state){

       case 'r':
	   if ( Is_usb_out_received(EP_HID_RX)){
//		   LED_Toggle(LED1);
		   Usb_reset_endpoint_fifo_access(EP_HID_RX);
		   data_length = Usb_byte_count(EP_HID_RX);
		   if (data_length > 2) data_length = 2;
		   usb_read_ep_rxpacket(EP_HID_RX, &usb_report[0], data_length, NULL);
		   Usb_ack_out_received_free(EP_HID_RX);

		   #if LCD_DISPLAY				// Multi-line LCD display
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
		   usb_state = 't';
	   }
	   break;

       case 't':

       if( Is_usb_in_ready(EP_HID_TX) )
       {
//    	  LED_Toggle(LED0);
          Usb_reset_endpoint_fifo_access(EP_HID_TX);
          
          //! Write report
          Usb_write_endpoint_data(EP_HID_TX, 8, usb_report[0]);
          Usb_write_endpoint_data(EP_HID_TX, 8, usb_report[1]);
          Usb_ack_in_ready_send(EP_HID_TX);
          usb_state = 'r';
       }
       break;
       }

#ifdef FREERTOS_USED
  }
#endif
}



#endif  // USB_DEVICE_FEATURE == ENABLED
