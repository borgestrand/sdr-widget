/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
** uac1_star_image.c
**
**  Created on: 2011-02-25
**      Author: Roger E Critchlow Jr, AD5DZ
*/

#include "image.h"

static void x_image_boot(void) {
}

static void x_image_init(void) {
}

static void x_image_task_init(void) {
}

image_t uac1_audio_image = {
	x_image_boot,
	x_image_init,
	x_image_task_init
};

image_t uac1_dg8saq_image = {
	x_image_boot,
	x_image_init,
	x_image_task_init
};

