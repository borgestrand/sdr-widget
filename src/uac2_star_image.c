/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
** uac2_star_image.c
**
**  Created on: 2011-02-25
**      Author: Roger E Critchlow Jr, AD5DZ
*/

#ifndef FREERTOS_USED
#if (defined __GNUC__)
#include "nlao_cpu.h"
#include "nlao_usart.h"
#endif
#else
#include <stdio.h>
#endif

#include "compiler.h"
#include "board.h"
#include "print_funcs.h"
#include "intc.h"
#include "pm.h"
#include "gpio.h"
#ifdef FREERTOS_USED
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "conf_usb.h"
#include "usb_task.h"
#if USB_DEVICE_FEATURE == ENABLED
#include "device_mouse_hid_task.h"
#endif
#if USB_HOST_FEATURE == ENABLED
//#include "host_keyboard_hid_task.h"
//#include "host_mouse_hid_task.h"
#endif
#include "composite_widget.h"
#include "taskAK5394A.h"
//#include "I2C.h"

/*
 * Task specific headers.
 */

#include "features.h"
#include "image.h"
#include "queue.h"
#include "taskEXERCISE.h"
#include "taskMoboCtrl.h"
#include "taskPowerDisplay.h"
#include "taskPushButtonMenu.h"
#include "device_audio_task.h"
#include "wdt.h"

#if LCD_DISPLAY				// Multi-line LCD display
#include "taskLCD.h"
#endif

static void x_image_boot(void) {
}

static void x_image_init(void) {
}

static void x_image_task_init(void) {
	// Initialize USB task
	usb_task_init();

#if USB_DEVICE_FEATURE == ENABLED
	mutexEP_IN = xSemaphoreCreateMutex(); // for co-ordinating multiple tasks using EP IN

#if LCD_DISPLAY						// Multi-line LCD display
	vStartTaskLCD();
	vStartTaskPowerDisplay();
	vStartTaskPushButtonMenu();
#endif
	vStartTaskMoboCtrl();
	vStartTaskEXERCISE( tskIDLE_PRIORITY );
	AK5394A_task_init();
	device_audio_task_init();
#endif
}

image_t uac2_audio_image = {
	x_image_boot,
	x_image_init,
	x_image_task_init
};

image_t uac2_dg8saq_image = {
	x_image_boot,
	x_image_init,
	x_image_task_init
};

