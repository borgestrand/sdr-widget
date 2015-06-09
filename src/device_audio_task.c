/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
#include "compiler.h"
#include "device_audio_task.h"
#include <stdint.h>

// To include definition of PS_USB_OFF
#include "Mobo_config.h"


//!
//! Public : (bit) mute
//! mute is set to TRUE when ACTIVE
//! mute is set to FALSE otherwise
//!/
volatile Bool mute, spk_mute;
volatile S32 FB_rate, FB_rate_initial, FB_rate_nominal; // BSB 20131031 FB_rate_initial and FB_rate_nominal added and changed to S32
S16 volume, spk_volume;
volatile Bool playerStarted = PS_USB_OFF; // BSB 20150516: changed into global variable, 20150609 changed definition


#if defined(HW_GEN_DIN10)		// BSB 20150501 global variable for input selector
volatile uint8_t input_select;
#endif

