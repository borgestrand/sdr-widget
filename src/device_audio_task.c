//* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
#include "compiler.h"
#include "device_audio_task.h"

//!
//! Public : (bit) mute
//! mute is set to TRUE when ACTIVE
//! mute is set to FALSE otherwise
//!/
volatile Bool mute, spk_mute;
volatile S32 FB_rate, FB_rate_initial, FB_rate_nominal; // BSB 20131031 FB_rate_initial and FB_rate_nominal added and changed to S32
S16 volume, spk_vol_usb_L, spk_vol_usb_R;				// BSB 20160320 Added stereo volume contr
S32 spk_vol_mult_L, spk_vol_mult_R;

volatile uint8_t input_select;							// BSB 20150501 global variable for input selector

#if defined(HW_GEN_DIN10)
volatile xSemaphoreHandle input_select_semphr = NULL; // BSB 20150626 audio channel selection semaphore
#endif

