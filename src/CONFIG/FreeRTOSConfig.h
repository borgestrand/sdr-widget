/* -*- mode: c; tab-width: 4; c-basic-offset: 4 -*- */
/* This header file is part of the ATMEL AVR32-SoftwareFramework-AT32UC3-1.5.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief FreeRTOS configuration file.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices can be used.
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
 *
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

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "board.h"


/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION      1
#define configUSE_IDLE_HOOK       0
#define configUSE_TICK_HOOK       0
#define configCPU_CLOCK_HZ        ( FCPU_HZ ) /* Hz clk gen */
#define configPBA_CLOCK_HZ        ( FPBA_HZ )
#define configTICK_RATE_HZ        ( ( portTickType ) 10000 )
#define configMAX_PRIORITIES      ( ( unsigned portBASE_TYPE ) 5 )
////////////////#define configMINIMAL_STACK_SIZE  ( ( unsigned portSHORT ) 128 )
#define configMINIMAL_STACK_SIZE  ( ( unsigned portSHORT ) 2048 )
//#define configMINIMAL_STACK_SIZE  ( ( unsigned portSHORT ) 1024 )
/* configTOTAL_HEAP_SIZE is not used when heap_3.c is used. */
#define configTOTAL_HEAP_SIZE     ( ( size_t ) ( 1024*50 ) )
#define configMAX_TASK_NAME_LEN   ( 20 )
#define configUSE_TRACE_FACILITY  0
#define configUSE_16_BIT_TICKS    0
#define configIDLE_SHOULD_YIELD   1
#define configUSE_MUTEXES		  1
/* Co-routine definitions. */
#define configUSE_CO_ROUTINES     0
#define configMAX_CO_ROUTINE_PRIORITIES ( 0 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskCleanUpResources       0
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_xTaskGetCurrentTaskHandle   0
#define INCLUDE_xTaskGetSchedulerState      0


/* configTICK_USE_TC is a boolean indicating whether to use a Timer Counter or
   the CPU Cycle Counter for the tick generation.
   Both methods will generate an accurate tick.
   0: Use of the CPU Cycle Counter.
   1: Use of the Timer Counter (configTICK_TC_CHANNEL is the TC channel). */
#define configTICK_USE_TC             0
#define configTICK_TC_CHANNEL         2

/* configHEAP_INIT is a boolean indicating whether to initialize the heap with
   0xA5 in order to be able to determine the maximal heap consumption. */
#define configHEAP_INIT               0

/* Debug trace configuration.
   configDBG is a boolean indicating whether to activate the debug trace. */
#define configDBG                     1
#define configDBG_USART               (&AVR32_USART1)
#define configDBG_USART_RX_PIN        AVR32_USART1_RXD_0_2_PIN
#define configDBG_USART_RX_FUNCTION   AVR32_USART1_RXD_0_2_FUNCTION
#define configDBG_USART_TX_PIN        AVR32_USART1_TXD_0_2_PIN
#define configDBG_USART_TX_FUNCTION   AVR32_USART1_TXD_0_2_FUNCTION
#define configDBG_USART_BAUDRATE      57600


/* USB task definitions. */
#define configTSK_USB_NAME                    ((const signed portCHAR *)"USB")
#define configTSK_USB_STACK_SIZE              256
#define configTSK_USB_PRIORITY                (tskIDLE_PRIORITY + 4)

/* USB device task definitions. */
#define configTSK_USB_DEV_NAME                ((const signed portCHAR *)"USB Device")
#define configTSK_USB_DEV_STACK_SIZE          256
#define configTSK_USB_DEV_PRIORITY            (tskIDLE_PRIORITY + 3)
#define UAC1_configTSK_USB_DEV_PERIOD              10
#define UAC2_configTSK_USB_DEV_PERIOD              2
#define HPSDR_configTSK_USB_DEV_PERIOD              2

/* USB host task definitions. */
#define configTSK_USB_HST_NAME                ((const signed portCHAR *)"USB Host")
#define configTSK_USB_HST_STACK_SIZE          256
#define configTSK_USB_HST_PRIORITY            (tskIDLE_PRIORITY + 2)
#define configTSK_USB_HST_PERIOD              200

/* USB device CDC task definitions. */
#define configTSK_USB_DCDC_NAME               ((const signed portCHAR *)"USB Device CDC")
#define configTSK_USB_DCDC_STACK_SIZE         256
#define configTSK_USB_DCDC_PRIORITY           (tskIDLE_PRIORITY + 1)
#define UAC1_configTSK_USB_DCDC_PERIOD             200
#define UAC2_configTSK_USB_DCDC_PERIOD             80
#define HPSDR_configTSK_USB_DCDC_PERIOD             80

/* USB device HID task definitions. */
#define configTSK_USB_DHID_MOUSE_NAME			((const signed portCHAR *)"USB Device Mouse HID")
#define configTSK_USB_DHID_MOUSE_STACK_SIZE		256
#define configTSK_USB_DHID_MOUSE_PRIORITY		(tskIDLE_PRIORITY + 1)
#define configTSK_USB_DHID_MOUSE_PERIOD			200

/* WM8845 task definitions. */
#define configTSK_WM8804_NAME					((const signed portCHAR *)"WM8804 Configuration")
#define configTSK_WM8804_STACK_SIZE				256
#define configTSK_WM8804_PRIORITY				(tskIDLE_PRIORITY + 1)
#define configTSK_WM8804_PERIOD					200

/* USB device Audio task definitions. */
#define configTSK_USB_DAUDIO_NAME				((const signed portCHAR *)"USB Device Audio")
#define configTSK_USB_DAUDIO_STACK_SIZE			256
#define configTSK_USB_DAUDIO_PRIORITY			(tskIDLE_PRIORITY + 3)
#define UAC1_configTSK_USB_DAUDIO_PERIOD		2
#define UAC2_configTSK_USB_DAUDIO_PERIOD		1
#define HPSDR_configTSK_USB_DAUDIO_PERIOD		2

/* AK5394A task definitions. */
#define configTSK_AK5394A_NAME					((const signed portCHAR *)"AK5394A") 
#define configTSK_AK5394A_STACK_SIZE			256
#define UAC1_configTSK_AK5394A_PRIORITY			(tskIDLE_PRIORITY + 2)// Was 1
#define UAC2_configTSK_AK5394A_PRIORITY			(tskIDLE_PRIORITY + 3)// Was +1, then +2
#define HPSDR_configTSK_AK5394A_PRIORITY		(tskIDLE_PRIORITY + 2)
#define UAC1_configTSK_AK5394A_PERIOD			50
#define UAC2_configTSK_AK5394A_PERIOD			50
#define HPSDR_configTSK_AK5394A_PERIOD			100

/* USB host Audio HID task definitions. */
#define configTSK_USB_HAUDIO_NAME             ((const signed portCHAR *)"USB Host Audio")
#define configTSK_USB_HAUDIO_STACK_SIZE       256
#define configTSK_USB_HAUDIO_PRIORITY         (tskIDLE_PRIORITY + 2)// Was 1
#define configTSK_USB_HAUDIO_PERIOD           10

/* taskMoboCtrl definitions. */
#define configTSK_MoboCtrl_NAME				  ((const signed portCHAR *)"taskMoboCtrl")
#define configTSK_MoboCtrl_STACK_SIZE		  1024
#define configTSK_MoboCtrl_PRIORITY			  (tskIDLE_PRIORITY + 1) // mobodebug: +2 works, but we prefer imrpoved UAC2 code // Was 0 // (tskIDLE_PRIORITY + 1) // Was 0
#define configTSK_MoboCtrl_PERIOD			  120

/* taskExercise definitions */
#define configTSK_EXERCISE_STACK_SIZE		256
#define configTSK_EXERCISE_PRIORITY			(tskIDLE_PRIORITY + 1 )
#define configTSK_EXERCISE_PERIOD			100	// 10ms

#endif /* FREERTOS_CONFIG_H */
