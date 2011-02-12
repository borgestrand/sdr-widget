/*
 * taskAK5394A.h
 *
 *  Created on: Feb 16, 2010
 *      Author: Alex
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
