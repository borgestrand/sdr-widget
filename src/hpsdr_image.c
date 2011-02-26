/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
** hpsdr_image.c
**
**  Created on: 2011-02-25
**      Author: Roger E Critchlow Jr, AD5DZ
*/

#include "image.h"

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
#include "hpsdr_taskAK5394A.h"
//#include "I2C.h"

/*
 * Task specific headers.
 */

#include "queue.h"
#include "taskEXERCISE.h"
#include "taskMoboCtrl.h"
#include "taskPowerDisplay.h"
#include "taskPushButtonMenu.h"
#include "device_audio_task.h"
#include "hpsdr_device_audio_task.h"
#include "wdt.h"

#if LCD_DISPLAY				// Multi-line LCD display
#include "taskLCD.h"
#endif

// image launch
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
  hpsdr_AK5394A_task_init();
  hpsdr_device_audio_task_init(HPSDR_EP_IQ_IN, HPSDR_EP_IQ_OUT, 0);
#endif
}

// descriptor accessors
static uint8_t *x_image_get_dev_desc_pointer(void) {
	return 0;
}
static uint16_t x_image_get_dev_desc_length(void) {
	return 0;
}
static uint8_t *x_image_get_conf_desc_pointer(void) {
	return 0;
}
static uint16_t x_image_get_conf_desc_length(void) {
	return 0;
}
static uint8_t *x_image_get_conf_desc_fs_pointer(void) {
	return 0;
}
static uint16_t x_image_get_conf_desc_fs_length(void) {
	return 0;
}
static uint8_t *x_image_get_conf_desc_hs_pointer(void) {
	return 0;
}
static uint16_t x_image_get_conf_desc_hs_length(void) {
	return 0;
}
static uint8_t *x_image_get_qualifier_desc_pointer(void) {
	return 0;
}
static uint16_t x_image_get_qualifier_desc_length(void) {
	return 0;
}

// specific request handlers
static void x_image_user_endpoint_init(uint8_t conf_nb) {
}
static Bool x_image_user_read_request(uint8_t type, uint8_t request) {
	return FALSE;
}
static void x_image_user_set_interface(U8 wIndex, U8 wValue) {
}

const image_t hpsdr_image = {
	x_image_boot,
	x_image_init,
	x_image_task_init,
	x_image_get_dev_desc_pointer,
	x_image_get_dev_desc_length,
	x_image_get_conf_desc_pointer,
	x_image_get_conf_desc_length,
	x_image_get_conf_desc_fs_pointer,
	x_image_get_conf_desc_fs_length,
	x_image_get_conf_desc_hs_pointer,
	x_image_get_conf_desc_hs_length,
	x_image_get_qualifier_desc_pointer,
	x_image_get_qualifier_desc_length,
	x_image_user_endpoint_init,
	x_image_user_read_request,
	x_image_user_set_interface
};

