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

extern uint32_t measured_Power(uint16_t voltage);
extern void 	vStartTaskMoboCtrl(void);

#endif
