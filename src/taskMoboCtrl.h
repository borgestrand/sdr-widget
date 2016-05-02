/*
 * taskMoboCtrl.h
 *
 *  Created on: 2010-06-13
 *      Author: Loftur Jonasson, TF3LJ
 */

#ifndef __TASKMOBOCTRL_H__
#define __TASKMOBOCTRL_H__

#include <stdint.h>

// SWR value x 100, in unsigned int format
extern uint16_t	measured_SWR;

extern uint16_t measured_Power(uint16_t voltage);
extern void 	vStartTaskMoboCtrl(void);



#ifdef HW_GEN_DIN20
// Control USB multiplexer in HW_GEN_DIN20
void mobo_usb_select(uint8_t USB_CH);

// Quick and dirty detect of whether front USB (A) is plugged in. No debounce here!
uint8_t mobo_usb_detect(void);
#endif



#endif
