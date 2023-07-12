/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * image.h
 *
 *  Created on: 2011-02-25
 *      Author: Roger E Critchlow Jr, AD5DZ
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include "compiler.h"
#include <stdint.h>

//
// the image defines the usb descriptor and
// all the components that implement the descriptor
//
// each defined image initializes this structure
// with pointers to its own functions for performing
// the necessary steps of bringing the image to life
//
typedef struct {
	// image launch
	void (*boot)(void);
	void (*init)(void);
	void (*task_init)(void);
	// descriptor access
	uint8_t *(*get_dev_desc_pointer)(void);
	uint16_t (*get_dev_desc_length)(void);
	uint8_t *(*get_conf_desc_pointer)(void);
	uint16_t (*get_conf_desc_length)(void);
	uint8_t *(*get_conf_desc_fs_pointer)(void);
	uint16_t (*get_conf_desc_fs_length)(void);
	uint8_t *(*get_conf_desc_hs_pointer)(void);
	uint16_t (*get_conf_desc_hs_length)(void);
	uint8_t *(*get_qualifier_desc_pointer)(void);
	uint16_t (*get_qualifier_desc_length)(void);
	// usb specific request handling
	void (*user_endpoint_init)(uint8_t conf_nb);
	Bool (*user_read_request)(uint8_t type, uint8_t request);
	void (*user_set_interface)(U8 wIndex, U8 wValue);
} image_t;

//
// these functions decide which image_t gets used,
// depending on the image feature selected, and
// uses it.
//
// image launch
extern void image_boot(void);
extern void image_init(void);
extern void image_task_init(void);
// descriptor access
extern uint8_t *image_get_dev_desc_pointer(void);
extern uint16_t image_get_dev_desc_length(void);
extern uint8_t *image_get_conf_desc_pointer(void);
extern uint16_t image_get_conf_desc_length(void);
extern uint8_t *image_get_conf_desc_fs_pointer(void);
extern uint16_t image_get_conf_desc_fs_length(void);
extern uint8_t *image_get_conf_desc_hs_pointer(void);
extern uint16_t image_get_conf_desc_hs_length(void);
extern uint8_t *image_get_qualifier_desc_pointer(void);
extern uint16_t image_get_qualifier_desc_length(void);
// usb specific request handling
extern void image_user_endpoint_init(uint8_t conf_nb);
extern Bool image_user_read_request(uint8_t type, uint8_t request);
extern void image_user_set_interface(U8 wIndex, U8 wValue);

//
// these are the image_t defined by the image specific
// source code.  This is "magically" kept in sync with
// the images defined in features.h by the efforts of
// the programmers who write this code.
//
extern const image_t uac1_audio_image;
extern const image_t uac2_audio_image;

#endif /* IMAGE_H_ */
