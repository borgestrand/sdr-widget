/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/*
 * taskAK5394A.h
 *
 *  Created on: Feb 16, 2010
 *  Refactored on: Feb 26, 2011
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
#define PDCA_CHANNEL_SSC_TX	   1
#define AUDIO_BUFFER_SIZE	(48*2*8) // 48 khz, stereo, 8 ms worth
#define SPK_BUFFER_SIZE 	(48*2*16)

//extern const gpio_map_t SSC_GPIO_MAP;
//extern const pdca_channel_options_t PDCA_OPTIONS;
//extern const pdca_channel_options_t SPK_PDCA_OPTIONS;

extern volatile U32 audio_buffer[2][AUDIO_BUFFER_SIZE];
extern volatile U32 spk_buffer[2][SPK_BUFFER_SIZE];
extern volatile avr32_ssc_t *ssc;
extern volatile int audio_buffer_in;
extern volatile int spk_buffer_out;

#define audio_buffer_0 audio_buffer[0]
#define audio_buffer_1 audio_buffer[1]
#define spk_buffer_0 spk_buffer[0]
#define spk_buffer_1 spk_buffer[1]

void AK5394A_pdca_disable(void);
void AK5394A_pdca_enable(void);
void AK5394A_task_init(Bool uac2);

#endif /* TASKAK5394A_H_ */
