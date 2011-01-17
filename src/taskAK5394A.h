/*
 * taskAK5394A.h
 *
 *  Created on: Feb 16, 2010
 *      Author: Alex
 *
 * Copyright (C) Alex Lee
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TASKAK5394A_H_
#define TASKAK5394A_H_


#define PDCA_CHANNEL_SSC_RX	   0	// highest priority of 8 channels
#define AUDIO_BUFFER_SIZE	(48*2*8) // 48 khz, stereo, 8 ms worth

extern volatile U32 audio_buffer_0[AUDIO_BUFFER_SIZE];
extern volatile U32 audio_buffer_1[AUDIO_BUFFER_SIZE];
extern volatile int audio_buffer_in;


void AK5394A_task_init(void);
void pdca_set_irq(void);

#endif /* TASKAK5394A_H_ */
