/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
** uac1_star_image.c
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
#include "wdt.h"

#if LCD_DISPLAY				// Multi-line LCD display
#include "taskLCD.h"
#endif

/*
** Image specific headers
*/
#include "usb_descriptors.h"
#include "uac1_usb_descriptors.h"
#include "usb_specific_request.h"
#include "uac1_usb_specific_request.h"
#include "uac1_device_audio_task.h"

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
  AK5394A_task_init();
  device_mouse_hid_task_init(UAC1_EP_HID_RX, UAC1_EP_HID_TX);
  uac1_device_audio_task_init(UAC1_EP_AUDIO_IN, UAC1_EP_AUDIO_OUT, UAC1_EP_AUDIO_OUT_FB);
#endif

}

// descriptor accessors
static uint8_t *x_audio_get_dev_desc_pointer(void) {
	return (uint8_t *)&uac1_audio_usb_dev_desc;
}
static uint16_t x_audio_get_dev_desc_length(void) {
	return (uint16_t)sizeof(uac1_audio_usb_dev_desc);
}
static uint8_t *x_dg8saq_get_dev_desc_pointer(void) {
	return (uint8_t *)&uac1_dg8saq_usb_dev_desc;
}
static uint16_t x_dg8saq_get_dev_desc_length(void) {
	return (uint16_t)sizeof(uac1_dg8saq_usb_dev_desc);
}
static uint8_t *x_image_get_conf_desc_pointer(void) {
	return (uint8_t *)&uac1_usb_conf_desc_fs;
}
static uint16_t x_image_get_conf_desc_length(void) {
	return sizeof(uac1_usb_conf_desc_fs);
}
static uint8_t *x_image_get_conf_desc_fs_pointer(void) {
	return (uint8_t *)&uac1_usb_conf_desc_fs;
}
static uint16_t x_image_get_conf_desc_fs_length(void) {
	return sizeof(uac1_usb_conf_desc_fs);
}
static uint8_t *x_image_get_conf_desc_hs_pointer(void) {
	return (uint8_t *)&uac1_usb_conf_desc_hs;
}
static uint16_t x_image_get_conf_desc_hs_length(void) {
	return sizeof(uac1_usb_conf_desc_hs);
}
static uint8_t *x_image_get_qualifier_desc_pointer(void) {
	return (uint8_t *)&uac1_usb_qualifier_desc;
}
static uint16_t x_image_get_qualifier_desc_length(void) {
	return sizeof(uac1_usb_qualifier_desc);
}

// specific request handlers
static void x_image_user_endpoint_init(uint8_t conf_nb) {
	uac1_user_endpoint_init(conf_nb);
}
static Bool x_image_user_read_request(uint8_t type, uint8_t request) {
	return uac1_user_read_request(type, request);
}
static void x_image_user_set_interface(U8 wIndex, U8 wValue) {
	uac1_user_set_interface(wIndex, wValue);
}

// image vector table
const image_t uac1_audio_image = {
	x_image_boot,
	x_image_init,
	x_image_task_init,
	x_audio_get_dev_desc_pointer,
	x_audio_get_dev_desc_length,
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

const image_t uac1_dg8saq_image = {
	x_image_boot,
	x_image_init,
	x_image_task_init,
	x_dg8saq_get_dev_desc_pointer,
	x_dg8saq_get_dev_desc_length,
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

