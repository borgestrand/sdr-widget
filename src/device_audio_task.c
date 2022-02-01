//* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
#include "compiler.h"
#include "device_audio_task.h"
#include "usb_specific_request.h"

//!
//! Public : (bit) mute
//! mute is set to TRUE when ACTIVE
//! mute is set to FALSE otherwise
//!/
volatile Bool mute, spk_mute;	// These variables are written to extensively but not heeded in playback
volatile uint8_t usb_spk_mute = 0; // This variable is written to by usb subsystem and heeded in playback

volatile S32 FB_rate, FB_rate_initial, FB_rate_nominal; // BSB 20131031 FB_rate_initial and FB_rate_nominal added and changed to S32
// With working volume flash:
// S16 spk_vol_usb_L = VOL_INVALID;			// BSB 20160320 Added stereo volume control
// S16 spk_vol_usb_R = VOL_INVALID;			// Not yet initialized from flash

// Without working volume flash;
S16 spk_vol_usb_L = VOL_DEFAULT;			// BSB 20160320 Added stereo volume control
S16 spk_vol_usb_R = VOL_DEFAULT;			// Forced to default value


S32 spk_vol_mult_L = 0;						// Full mute for now, re-formated in uac?_device_audio_task_init
S32 spk_vol_mult_R = 0;

volatile uint8_t input_select;				// BSB 20150501 global variable for input selector


// RXMODFIX Global variables for tuning scanning algorithm. Optimized for warm wm8804
volatile uint8_t wm8804_LINK_MAX_ATTEMPTS = 0x64;//  0x3d;	// 29 Results after 1st optimization - increase all three to lower risk of noise at cost of longer scan times 
volatile uint8_t wm8804_LINK_DETECTS_OK = 0x08;		// 05 Results after 1st optimization 
volatile uint8_t wm8804_TRANS_ERR_FAILURE = 0x1d;	// 14 Results after 1st optimization - 3d081d testing device "3" with slowest WM8804 to date



#if (defined HW_GEN_DIN20) || (defined HW_GEN_RXMOD)
volatile uint8_t usb_ch;					// Front or rear USB channel
volatile uint8_t usb_ch_swap;				// USB channel is about to swap!
#endif

#if (defined HW_GEN_DIN10) || (defined HW_GEN_DIN20) || (defined HW_GEN_RXMOD)
volatile xSemaphoreHandle input_select_semphr = NULL; // BSB 20150626 audio channel selection semaphore
#endif

#if (defined HW_GEN_RXMOD)
volatile xSemaphoreHandle I2C_busy = NULL; 
#endif
