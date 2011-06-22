/**
* @file sdr1000io.c
* @brief Audio I/O for sdr1000
* @author John Melton, G0ORX/N6LYT
* @version 0.1
* @date 2009-10-13
*/


/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "sdr1000.h"
#include "sdr1000io.h"

#ifdef PORTAUDIO
#include <portaudio.h>
#else
#include <linux/soundcard.h>
#endif


#define SAMPLE_RATE 48000   /* the sampling rate */
#define CHANNELS 2  /* 1 = mono 2 = stereo */
#define SAMPLES_PER_BUFFER 1024

#ifndef PORTAUDIO
#define SAMPLE_SIZE 16
#endif


#ifdef PORTAUDIO
static PaStream* stream;
#else
static int fd;
#endif


int sdr1000_open(void) {
    int arg;
    int status;
    int rc;
#ifdef PORTAUDIO
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;
    const PaStreamInfo *info;
    int devices;
    int i;
    const PaDeviceInfo* deviceInfo;

fprintf(stderr,"sdr1000_open: portaudio\n");
#else
fprintf(stderr,"sdr1000_open: %s\n",sdr1000_get_device());
#endif


#ifdef PORTAUDIO
    rc=Pa_Initialize();
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_Initialize failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }

    devices=Pa_GetDeviceCount();
    if(devices<0) {
        fprintf(stderr,"Px_GetDeviceCount failed: %s\n",Pa_GetErrorText(devices));
    } else {
        fprintf(stderr,"default input=%d output=%d devices=%d\n",Pa_GetDefaultInputDevice(),Pa_GetDefaultOutputDevice(),devices);

        for(i=0;i<devices;i++) {
            deviceInfo=Pa_GetDeviceInfo(i);
            fprintf(stderr,"%d - %s\n",i,deviceInfo->name);
                fprintf(stderr,"maxInputChannels: %d\n",deviceInfo->maxInputChannels);
                fprintf(stderr,"maxOututChannels: %d\n",deviceInfo->maxOutputChannels);
                //fprintf(stderr,"defaultLowInputLatency: %f\n",deviceInfo->defaultLowInputLatency);
                //fprintf(stderr,"defaultLowOutputLatency: %f\n",deviceInfo->defaultLowOutputLatency);
                //fprintf(stderr,"defaultHighInputLatency: %f\n",deviceInfo->defaultHighInputLatency);
                //fprintf(stderr,"defaultHighOutputLatency: %f\n",deviceInfo->defaultHighOutputLatency);
                //fprintf(stderr,"defaultSampleRate: %f\n",deviceInfo->defaultSampleRate);
        }
    }

    inputParameters.device=atoi(sdr1000_get_input());
    inputParameters.channelCount=2;
    inputParameters.sampleFormat=paFloat32;
    inputParameters.suggestedLatency=Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo=NULL;

    outputParameters.device=atoi(sdr1000_get_output());
    outputParameters.channelCount=2;
    outputParameters.sampleFormat=paFloat32;
    outputParameters.suggestedLatency=Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo=NULL;

fprintf(stderr,"input device=%d output device=%d\n",inputParameters.device,outputParameters.device);
    rc=Pa_OpenStream(&stream,&inputParameters,&outputParameters,(double)sdr1000_get_sample_rate(),(unsigned long)SAMPLES_PER_BUFFER,paNoFlag,NULL,NULL);
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_OpenStream failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }

    rc=Pa_StartStream(stream);
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_StartStream failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }

    info=Pa_GetStreamInfo(stream);
    if(info!=NULL) {
        fprintf(stderr,"stream.sampleRate=%f\n",info->sampleRate);
        fprintf(stderr,"stream.inputLatency=%f\n",info->inputLatency);
        fprintf(stderr,"stream.outputLatency=%f\n",info->outputLatency);
    } else {
        fprintf(stderr,"Pa_GetStreamInfo returned NULL\n");
    }

#else
    /* open sound device */
    fd = open(sdr1000_get_device(), O_RDWR);
    if (fd < 0) {
        perror("open of audio device failed");
        exit(1);
    }

    /* set sampling parameters */
    arg = SAMPLE_SIZE;      /* sample size */
    status = ioctl(fd, SOUND_PCM_WRITE_BITS, &arg);
    if (status == -1)
        perror("SOUND_PCM_WRITE_BITS ioctl failed");
    if (arg != SAMPLE_SIZE)
        perror("unable to set write sample size");

    status = ioctl(fd, SOUND_PCM_READ_BITS, &arg);
    if (status == -1)
        perror("SOUND_PCM_READ_BITS ioctl failed");
    if (arg != SAMPLE_SIZE)
        perror("unable to set read sample size");

    arg = CHANNELS;  /* mono or stereo */
    status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &arg);
    if (status == -1)
        perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
    if (arg != CHANNELS)
        perror("unable to set number of channels");

    arg = sdr1000_get_sample_rate();      /* sampling rate */
fprintf(stderr,"sample_rate: %d\n",arg);
    status = ioctl(fd, SOUND_PCM_WRITE_RATE, &arg);
    if (status == -1)
        perror("SOUND_PCM_WRITE_WRITE ioctl failed");

    arg = AFMT_S16_LE;       /* signed little endian */
    status = ioctl(fd, SOUND_PCM_SETFMT, &arg);
    if (status == -1)
        perror("SOUND_PCM_SETFMTS ioctl failed");

#endif

    return 0;
}

int sdr1000_close() {
#ifdef PORTAUDIO
    int rc=Pa_Terminate();
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_Terminate failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }
#else
    close(fd);
#endif
}

#ifdef PORTAUDIO
int sdr1000_write(float* left_samples,float* right_samples) {
    int rc;
    int i;
    float audio_buffer[SAMPLES_PER_BUFFER*2];

    rc=0;

    // interleave samples
    for(i=0;i<SAMPLES_PER_BUFFER;i++) {
        audio_buffer[i*2]=right_samples[i];
        audio_buffer[(i*2)+1]=left_samples[i];
    }

    //fprintf(stderr,"write available=%ld\n",Pa_GetStreamWriteAvailable(stream));
    rc=Pa_WriteStream(stream,audio_buffer,SAMPLES_PER_BUFFER);
    if(rc!=0) {
        fprintf(stderr,"error writing audio_buffer %s (rc=%d)\n",Pa_GetErrorText(rc),rc);
    }


    return rc;
}

int sdr1000_read(float* left_samples,float* right_samples) {
    int rc;
    int i;
    float audio_buffer[SAMPLES_PER_BUFFER*2];

    //fprintf(stderr,"read available=%ld\n",Pa_GetStreamReadAvailable(stream));
    rc=Pa_ReadStream(stream,audio_buffer,SAMPLES_PER_BUFFER);
    if(rc!=0) {
        fprintf(stderr,"error reading audio_buffer %s (rc=%d)\n",Pa_GetErrorText(rc),rc);
    }

    // de-interleave samples
    for(i=0;i<SAMPLES_PER_BUFFER;i++) {
        left_samples[i]=audio_buffer[i*2];
        right_samples[i]=audio_buffer[(i*2)+1];
//fprintf(stderr,"%d left=%f right=%f\n",i, left_samples[i],right_samples[i]);
    }

    return rc;
}
#else
int sdr1000_write(unsigned char* buffer,int buffer_size) {
    int rc;
    int bytes;

    rc = write(fd,buffer,buffer_size);
    if(rc!=buffer_size) {
        perror("error reading audio buffer");
        exit(1);
    }

    return rc;
}

int sdr1000_read(unsigned char* buffer,int buffer_size) {
    int rc;
    int bytes;

    rc = read(fd,buffer,buffer_size);
    if(rc!=buffer_size) {
        perror("error reading audio buffer");
        exit(1);
    }

    return rc;
}
#endif
