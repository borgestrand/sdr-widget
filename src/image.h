/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * image.h
 *
 *  Created on: 2011-02-25
 *      Author: Roger E Critchlow Jr, AD5DZ
 */

#ifndef IMAGE_H_
#define IMAGE_H_

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
  void (*image_boot)(void);
  void (*image_init)(void);
  void (*image_task_init)(void);
} image_t;

//
// these functions decide which image_t gets used,
// depending on the image feature selected, and
// uses it.
//
extern void image_boot(void);
extern void image_init(void);
extern void image_task_init(void);

//
// these are the image_t defined by the image specific
// source code.  This is "magically" kept in sync with
// the images defined in features.h by the efforts of
// the programmers who write this code.
//
extern image_t flashyblinky_image;
extern image_t uac1_audio_image;
extern image_t uac1_dg8saq_image;
extern image_t uac2_audio_image;
extern image_t uac2_dg8saq_image;
extern image_t hpsdr_image;

#endif /* IMAGE_H_ */
