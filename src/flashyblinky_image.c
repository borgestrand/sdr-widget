/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
** flashyblinky_image.c
**
**  Created on: 2011-02-25
**      Author: Roger E Critchlow Jr, AD5DZ
*/

#include "image.h"

// image launch
static void x_image_boot(void) {
}

static void x_image_init(void) {
}

static void x_image_task_init(void) {
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
#if USB_HIGH_SPEED_SUPPORT==ENABLED
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
#endif
// specific request handlers
static void x_image_user_endpoint_init(uint8_t conf_nb) {
}
static Bool x_image_user_read_request(uint8_t type, uint8_t request) {
	return FALSE;
}
static void x_image_user_set_interface(U8 wIndex, U8 wValue) {
}

// image vector table
const image_t flashyblinky_image = {
	x_image_boot,
	x_image_init,
	x_image_task_init,
	x_image_get_dev_desc_pointer,
	x_image_get_dev_desc_length,
	x_image_get_conf_desc_pointer,
	x_image_get_conf_desc_length,
	x_image_get_conf_desc_fs_pointer,
	x_image_get_conf_desc_fs_length,
#if USB_HIGH_SPEED_SUPPORT==ENABLED
	x_image_get_conf_desc_hs_pointer,
	x_image_get_conf_desc_hs_length,
	x_image_get_qualifier_desc_pointer,
	x_image_get_qualifier_desc_length,
#else
	x_image_get_conf_desc_fs_pointer,
	x_image_get_conf_desc_fs_length,
	NULL,
	NULL,
#endif
	x_image_user_endpoint_init,
	x_image_user_read_request,
	x_image_user_set_interface
};

