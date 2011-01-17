/*
 * rotary_encoder.h
 *
 * \brief Rotary Encoder driven Dual Speed VFO and Menu selector, driven by
 * two Interrupt inputs, NMI and EXT_INT7.  Three encoder outputs provided
 * in the form of global (voaltile) values:
 *
 * (int32_t) freq_delta_from_enc is an incremental value to be added directly to
 * the uint32_t value representing the Si570 frequency.  One revolution of the
 * Encoder corresponds to an increment of 4kHz (Si570 frequency is divided by 4).
 * If the Encoder is turned for more than 1/5th of a turn at an angular speed
 * greater than defined in rotary_encoder.h, then the faster rate of the dual
 * speed function kicks in, and each revolution of the Encoder results in an
 * increment of 100 times the normal rate.  This function becomes inactive in
 * 1/3 of a second, or if briefly turned in the opposite direction, as defined
 * in rotary_encoder.h.
 * Associated with freq_delta_from_enc, there is also the (volatile) bool
 * FRQ_fromenc, indicating whether a new value is available as a result of turning
 * the encoder.
 *
 * (int8_t) menu_steps_from_enc is an incremental value which increases by 10 for
 * each turn of the Rotary Encoder.  This value is used by the Menu function in
 * taskPushButtonMenu.c.
 * Associated menu_steps_from_enc, there is also the (volatile) bool MENU_fromenc,
 * indicating whether a new value is available as a result of turning the encoder.
 *
 * (int16_t) val_steps_from_enc is an incremental value which increases by 100 for
 * each turn of the Rotary Encoder.  This value is used by the Menu function in
 * taskPushButtonMenu.c
 * Associated val_steps_from_enc, there is also the (volatile) bool VALUE_fromenc,
 * indicating whether a new value is available as a result of turning the encoder.
 *
 *  Created on: 2010-06-10
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef ROTARY_ENCODER_H_
#define ROTARY_ENCODER_H_

// DEFS for the Rotary Encoder VFO function
#define ENC_PULSES			512					// Number of resolvable Encoder States per revolution.
												// Note that the pulses per revolution parameter is not consistent
												// for all encoders.  For some Rotary Encoders "pulses per revolution"
												// indicates the number of pulses per revolution for each of the two
												// phase outputs, while for others, this parameter indicates the total
												// number of resolvable states.  In case of the former, the total number
												// of resolvable states is 4 times the specified pulses per revolution.
#define	ENC_RESOLUTION		(1<<23)/1000		// Resolution in increment for a full kiloHertz (2^23/1kHz), used to achieve 1kHz per
												// revolution of the VFO (Rotary Encoder) knob.

// Definitions for Variable Speed Rotary Encoder function
// The below parameters are hopefully more or less generic for any encoder, down to 32 pulses or so,
// but are optimized for a 1024 state Rotary Encoder
#define ENC_RTC_RATE		115000				// Rate of the Real Time Clock used to measure Angular Speed.
#define ENC_FAST_ANGLE		(72.0/360.0)		// (1/5) Minimum travel angle for Encoder Fast Mode
#define ENC_FAST_REV		(ENC_RTC_RATE/4)	// RTC_rate/Revolutions (4) for Encoder Fast Mode
#define ENC_FAST_MULTIPLY	100					// Encoder click multiplier during FAST mode
#define ENC_FAST_PATIENCE	5*(ENC_RTC_RATE/10)	// Time in 1/10th of seconds.  Time to revert to
												// normal encoder mode if no encoder activity

extern volatile int32_t 	freq_delta_from_enc;// Pass accumulated frequency delta from Rotary Interrupt function
extern volatile int8_t 		menu_steps_from_enc;// 10 steps per rev, accumulated delta from Rotary Encoder, used by Menu function
extern volatile int16_t		val_steps_from_enc;	// 100 steps per rev, accumulated delta from Rotary Encoder, used by Menu funciton
extern volatile bool		FRQ_fromenc;		// Flag: New frequency delta ready from Rotary Interrupt function
extern volatile bool		MENU_fromenc;		// Flag: New Menu selection ready from Rotary Interrupt function
extern volatile bool		VAL_fromenc;		// Flag: New Value available from Rotary Interrupt function
extern void 				encoder_init(void);	// Encoder Initialise, called by external function

#endif /* ROTARY_ENCODER_H_ */
