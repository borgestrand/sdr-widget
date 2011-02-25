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
static image_t *image;

// boot gets called before anything
// initializes to setup whatever
// needs setting before initialization
void image_boot(void) {
  if ( FEATURE_IMAGE_FLASHYBLINKY ) image = &flashyblinky_image;
  else if ( FEATURE_IMAGE_UAC1_AUDIO ) image = &uac1_audio_image;
  else if ( FEATURE_IMAGE_UAC1_DG8SAQ ) image = &uac1_dg8saq_image;
  else if ( FEATURE_IMAGE_UAC2_AUDIO ) image = &uac2_audio_image;
  else if ( FEATURE_IMAGE_UAC2_DG8SAQ ) image = &uac2_dg8saq_image;
  else if ( FEATURE_IMAGE_HPSDR) image = &hpsdr_image;
  else image = &uac1_audio_image;
  image->image_boot();
}

// init gets called when the board has
// completed the boot.  It typically
// calls the init functions of the 
// image components
void image_init(void) {
  image->image_init();
}

// task_init gets called when we're ready
// to start the various RTOS tasks running
// it typically calls the task init functions
// of the image components.
void image_task_init(void) {
  image->image_task_init();
}
