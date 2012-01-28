/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * image.c
 *
 *  Created on: 2011-02-25
 *      Author: Roger E Critchlow Jr, AD5DZ
 */

#include <stdint.h>

#include "features.h"
#include "image.h"

// this pointer gets set to the correct image_t
static const image_t *image;

// boot gets called before anything
// initializes to setup whatever
// needs setting before initialization
void image_boot(void) {
  if ( FEATURE_IMAGE_UAC1_AUDIO ) image = &uac1_audio_image;
  else if ( FEATURE_IMAGE_UAC1_DG8SAQ ) image = &uac1_dg8saq_image;
  else if ( FEATURE_IMAGE_UAC2_AUDIO ) image = &uac2_audio_image;
  else if ( FEATURE_IMAGE_UAC2_DG8SAQ ) image = &uac2_dg8saq_image;
  else if ( FEATURE_IMAGE_HPSDR) image = &hpsdr_image;
  else image = &uac1_audio_image;
  image->boot();
}

// init gets called when the board has
// completed the boot.  It typically
// calls the init functions of the 
// image components
void image_init(void) {
  image->init();
}

// task_init gets called when we're ready
// to start the various RTOS tasks running
// it typically calls the task init functions
// of the image components.
void image_task_init(void) {
  image->task_init();
}

// descriptor access
uint8_t *image_get_dev_desc_pointer(void) {
	return image->get_dev_desc_pointer();
}
uint16_t image_get_dev_desc_length(void) {
	return image->get_dev_desc_length();
}
uint8_t *image_get_conf_desc_pointer(void) {
	return image->get_conf_desc_pointer();
}
uint16_t image_get_conf_desc_length(void) {
	return image->get_conf_desc_length();
}
uint8_t *image_get_conf_desc_fs_pointer(void) {
	return image->get_conf_desc_fs_pointer();
}
uint16_t image_get_conf_desc_fs_length(void) {
	return image->get_conf_desc_fs_length();
}
uint8_t *image_get_conf_desc_hs_pointer(void) {
	return image->get_conf_desc_hs_pointer();
}
uint16_t image_get_conf_desc_hs_length(void) {
	return image->get_conf_desc_hs_length();
}
uint8_t *image_get_qualifier_desc_pointer(void) {
	return image->get_qualifier_desc_pointer();
}
uint16_t image_get_qualifier_desc_length(void) {
	return image->get_qualifier_desc_length();
}
// usb specific request handling
void image_user_endpoint_init(uint8_t conf_nb) {
	image->user_endpoint_init(conf_nb);
}
Bool image_user_read_request(uint8_t type, uint8_t request) {
	return image->user_read_request(type, request);
}
void image_user_set_interface(U8 wIndex, U8 wValue) {
	image->user_set_interface(wIndex, wValue);
}
