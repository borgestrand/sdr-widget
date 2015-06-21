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
volatile uint8_t playerStarted = PS_USB_OFF; // BSB 20150516: changed into global uint8_t variable, 20150609 changed definition
volatile uint32_t silence_USB = SILENCE_USB_LIMIT;	// BSB 20150621: detect silence in USB channel, initially assume silence
volatile uint16_t wm8805_zerotimer = SILENCE_WM_LIMIT;			// Initially assume WM8805 is silent
volatile uint16_t wm8805_loudtimer = LOUD_WM_INIT;				// Initially assume WM8805 is silent



#if defined(HW_GEN_DIN10)		// BSB 20150501 global variable for input selector
volatile uint8_t input_select;
#endif

