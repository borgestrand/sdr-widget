/*
 *  audiostream.c
 *  HPSDR
 *
 *  Created by John Melton on 10/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <CoreFoundation/CFSocket.h>
#include <AudioToolbox/AudioQueue.h>


#include "audiostream.h"

unsigned char audiostream_buffer[AUDIO_BUFFER_SIZE];

AQCallbackStruct aqc;
int buffer=0;
int started=0;


//---------------------------------------------
//
// Audio Queue
//
//---------------------------------------------

static void aqOutputCallback(void *aqCustomData,AudioQueueRef aqRef,AudioQueueBufferRef aqBufferRef) {
	
	//fprintf(stderr,"aqOutputCallback %d\n",aqBufferRef->mAudioDataBytesCapacity);
	//memcpy(aqBufferRef->mAudioData,audiostream_buffer,aqBufferRef->mAudioDataBytesCapacity);
	//AudioQueueEnqueueBuffer(aqRef,aqBufferRef,0,NULL);

}

void processAudioBuffer(unsigned char* audiostream_buffer) {
	int err;
	memcpy(aqc.mBuffers[buffer]->mAudioData,audiostream_buffer,AUDIO_BUFFER_SIZE);
	aqc.mBuffers[buffer]->mAudioDataByteSize=AUDIO_BUFFER_SIZE;
	err=AudioQueueEnqueueBuffer(aqc.queue,aqc.mBuffers[buffer],0,NULL);
	if(err) {
		fprintf(stderr,"AudioQueueEnqueueBuffer error=%d\n",err);
	}
	buffer++;
	if(buffer>=AUDIO_BUFFERS) {
		
		if(started==0) {
			fprintf(stderr,"AudioQueueStart\n");
			err = AudioQueueStart(aqc.queue, NULL);
			if(err) {
				fprintf(stderr,"AudioQueueStart: err=%d\n",err);
			}
			started=1;
		}
		buffer=0;
	}
}

void audiostream_init() {
	
	int i;

    aqc.mDataFormat.mSampleRate = 8000.0;
	//aqc.mDataFormat.mFormatID = kAudioFormatLinearPCM;
	aqc.mDataFormat.mFormatID = kAudioFormatALaw;
	
	//aqc.mDataFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger|kAudioFormatFlagIsPacked;
	aqc.mDataFormat.mFormatFlags = kAudioFormatFlagIsPacked;
	aqc.mDataFormat.mBytesPerPacket = 1;
    aqc.mDataFormat.mFramesPerPacket = 1;
    aqc.mDataFormat.mBytesPerFrame = 1;
    aqc.mDataFormat.mChannelsPerFrame = 1;
    aqc.mDataFormat.mBitsPerChannel = 8;
    aqc.frameCount = AUDIO_BUFFER_SIZE;
	
	UInt32 err = AudioQueueNewOutput(&aqc.mDataFormat,
							  aqOutputCallback,
							  &aqc,
							  NULL,
							  NULL,
							  0,
							  &aqc.queue);
    if (err) {
		fprintf(stderr,"AudioQueueNewOutput: err=%ld\n",err);
	}

	for (i=0; i<AUDIO_BUFFERS; i++) {
		//fprintf(stderr,"AudioQueueAllocateBuffer\n");
        err = AudioQueueAllocateBuffer(aqc.queue, AUDIO_BUFFER_SIZE,
									   &aqc.mBuffers[i]);
        if (err) {
			fprintf(stderr,"AudioQueueAllocateBuffer: err=%ld\n",err);
		}
	
    }
	
	buffer=0;
	started=0;
		
}