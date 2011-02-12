/*
 * taskAK5394A.h
 *
 *  Created on: Feb 16, 2010
 *      Author: Alex
 */

#ifndef TASKAK5394A_H_
#define TASKAK5394A_H_


#define PDCA_CHANNEL_SSC_RX	   0	// highest priority of 8 channels
#define PDCA_CHANNEL_SSC_TX	   1
#define AUDIO_BUFFER_SIZE	(48*2*8) // 48 khz, stereo, 8 ms worth
#define SPK_BUFFER_SIZE 	(48*2*16)

extern volatile U32 audio_buffer_0[AUDIO_BUFFER_SIZE];
extern volatile U32 audio_buffer_1[AUDIO_BUFFER_SIZE];
extern volatile U32 spk_buffer_0[SPK_BUFFER_SIZE];
extern volatile U32 spk_buffer_1[SPK_BUFFER_SIZE];
extern volatile int audio_buffer_in;
extern volatile int spk_buffer_out;


void AK5394A_task_init(void);
void pdca_set_irq(void);

#endif /* TASKAK5394A_H_ */
