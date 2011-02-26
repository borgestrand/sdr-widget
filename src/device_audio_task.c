/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
#include "compiler.h"
#include "device_audio_task.h"

//!
//! Public : (bit) mute
//! mute is set to TRUE when ACTIVE
//! mute is set to FALSE otherwise
//!/
volatile Bool mute, spk_mute;
volatile U32 FB_rate;
S16 volume, spk_volume;
