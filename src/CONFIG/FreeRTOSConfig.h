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
#define configTSK_USB_DEV_PERIOD              2


/* USB device CDC task definitions. */
#define configTSK_USB_DCDC_NAME               ((const signed portCHAR *)"USB Device CDC")
#define configTSK_USB_DCDC_STACK_SIZE         256
#define configTSK_USB_DCDC_PRIORITY           (tskIDLE_PRIORITY + 1)
#define configTSK_USB_DCDC_PERIOD             80


/* USB device Audio task definitions. */
#define configTSK_USB_DAUDIO_NAME             ((const signed portCHAR *)"USB Device Audio")
#define configTSK_USB_DAUDIO_STACK_SIZE       256
#define configTSK_USB_DAUDIO_PRIORITY         (tskIDLE_PRIORITY + 2)
#define configTSK_USB_DAUDIO_PERIOD           2

/* AK5394A task definitions. */
#define configTSK_AK5394A_NAME             	((const signed portCHAR *)"AK5394A")
#define configTSK_AK5394A_STACK_SIZE       	256
#define configTSK_AK5394A_PRIORITY         	(tskIDLE_PRIORITY + 2)
#define configTSK_AK5394A_PERIOD           	100


/* taskMoboCtrl definitions. */
#define configTSK_MoboCtrl_NAME				  ((const signed portCHAR *)"taskMoboCtrl")
#define configTSK_MoboCtrl_STACK_SIZE		  1024
#define configTSK_MoboCtrl_PRIORITY			  (tskIDLE_PRIORITY + 1)
// Not used... is in a loop with a fixed wait of 10ms at the end
//#define configTSK_MoboCtrl_PERIOD			  100

/* taskPowerDisplay definitions. */
#define configTSK_PDISPLAY_NAME				  ((const signed portCHAR *)"taskPowerDisplay")
#define configTSK_PDISPLAY_STACK_SIZE		  1024
#define configTSK_PDISPLAY_PRIORITY			  (tskIDLE_PRIORITY )
// Not used... Lowest priority task, but is in a fast loop with a fixed wait of only 5ms at the end
//#define configTSK_PDISPLAY_PERIOD			  50

/* taskPushButtonMenu definitions. */
#define configTSK_PBTNMENU_NAME				  ((const signed portCHAR *)"taskPushButtonMenu")
#define configTSK_PBTNMENU_STACK_SIZE		  1024
#define configTSK_PBTNMENU_PRIORITY			  (tskIDLE_PRIORITY )
#define configTSK_PBTNMENU_PERIOD			  100	// 10ms

/* taskLCD definitions */
// Priority has to be same or greater than that of client tasks such as MoboControl and PowerDisplay
#define configTSK_LCD_PRIORITY        		( tskIDLE_PRIORITY + 1)
#define	configTSK_LCD_STACK_SIZE			1024

/* taskExercise definitions */
#define configTSK_EXERCISE_STACK_SIZE		256
#define configTSK_EXERCISE_PRIORITY			(tskIDLE_PRIORITY + 1 )
#define configTSK_EXERCISE_PERIOD			100	// 10ms

#endif /* FREERTOS_CONFIG_H */
