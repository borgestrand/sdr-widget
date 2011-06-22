/**
* @file ozyio.c
* @brief USB I/O with Ozy
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
#include <sys/timeb.h>

#include "softrock.h"
#include "softrockio.h"

#ifdef PULSEAUDIO
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#endif
#ifdef PORTAUDIO
#include <portaudio.h>
#endif
#ifdef DIRECTAUDIO
#include <linux/soundcard.h>
#endif

static int timing=0;
static struct timeb start_time;
static struct timeb end_time;
static int sample_count=0;


#define SAMPLE_RATE 48000   /* the sampling rate */
#define CHANNELS 2  /* 1 = mono 2 = stereo */
#define SAMPLES_PER_BUFFER 1024

#ifdef DIRECTAUDIO
#define SAMPLE_SIZE 16
#endif


#ifdef PULSEAUDIO
static pa_simple* stream;
#endif
#ifdef PORTAUDIO
static PaStream* stream;
#endif
#ifdef DIRECTAUDIO
static int fd;
#endif

int softrock_open(void) {
    int arg;
    int status;
    int rc;
#ifdef PULSEAUDIO
    int error;
    pa_sample_spec params; 
    pa_buffer_attr attrs;
#endif
#ifdef PORTAUDIO
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;
    PaStreamInfo *info;
    int devices;
    int i;
    PaDeviceInfo* deviceInfo;
fprintf(stderr,"softrock_open: portaudio\n");
#endif
#ifdef DIRECTAUDIO
fprintf(stderr,"softrock_open: %s\n",softrock_get_device());
#endif

    if(softrock_get_playback()) {
        return 0;
    }

#ifdef PULSEAUDIO
    fprintf(stderr,"Using PulseAudio\n");

    params.format=PA_SAMPLE_FLOAT32LE;
    params.rate=softrock_get_sample_rate();
    params.channels=2;


    attrs.maxlength=attrs.minreq=attrs.prebuf=attrs.tlength=(uint32_t)-1;
    attrs.fragsize=attrs.maxlength=attrs.minreq=attrs.prebuf=(uint32_t)-1;
    attrs.fragsize=SAMPLES_PER_BUFFER*2 * sizeof(float);
    attrs.tlength=SAMPLES_PER_BUFFER*2 * sizeof(float);

fprintf(stderr,"params.rate=%d\n",params.rate);

    stream=pa_simple_new("localhost","Softrock", PA_STREAM_RECORD, NULL, "IQ", &params, NULL, &attrs, &error);
    if(stream==NULL) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        exit(0);
    }
    ftime(&start_time);
#endif
#ifdef PORTAUDIO
    fprintf(stderr,"Using PortAudio\n");

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

    inputParameters.device=atoi(softrock_get_input());
    inputParameters.channelCount=2;
    inputParameters.sampleFormat=paFloat32;
    inputParameters.suggestedLatency=Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo=NULL;

    outputParameters.device=atoi(softrock_get_output());
    outputParameters.channelCount=2;
    outputParameters.sampleFormat=paFloat32;
    outputParameters.suggestedLatency=Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo=NULL;

fprintf(stderr,"input device=%d output device=%d\n",inputParameters.device,outputParameters.device);
    rc=Pa_OpenStream(&stream,&inputParameters,&outputParameters,(double)softrock_get_sample_rate(),(unsigned long)SAMPLES_PER_BUFFER,paNoFlag,NULL,NULL);
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
#endif
#ifdef DIRECTAUDIO

    fprintf(stderr,"Using direct audio\n");
    /* open sound device */
    fd = open(softrock_get_device(), O_RDWR);
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

    arg = softrock_get_sample_rate();      /* sampling rate */
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

int softrock_close() {

    if(softrock_get_playback()) {
        return 0;
    }

#ifdef PULSEAUDIO
    pa_simple_free(stream);
#endif
#ifdef PORTAUDIO
    int rc=Pa_Terminate();
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_Terminate failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }
#endif
#ifdef DIRECTAUDIO
    close(fd);
#endif
    return 0;
}

#ifdef PULSEAUDIO
int softrock_write(float* left_samples,float* right_samples) {
    int rc;
    int i;
    float audio_buffer[SAMPLES_PER_BUFFER*2];

    rc=0;

    // interleave samples
    for(i=0;i<SAMPLES_PER_BUFFER;i++) {
        audio_buffer[i*2]=right_samples[i];
        audio_buffer[(i*2)+1]=left_samples[i];
    }

    // need to add writing
    return rc;
}

int softrock_read(float* left_samples,float* right_samples) {
    int rc;
    int error;
    int i;
    float audio_buffer[SAMPLES_PER_BUFFER*2];


    if(softrock_get_playback()) {
        softrock_playback_buffer(audio_buffer,sizeof(audio_buffer));
    } else {
        //fprintf(stderr,"read available=%ld\n",Pa_GetStreamReadAvailable(stream));
        //ftime(&start_time);
        rc=pa_simple_read(stream,&audio_buffer[0],sizeof(audio_buffer),&error);
        if(rc<0) {
            fprintf(stderr,"error reading audio_buffer %s (rc=%d)\n", pa_strerror(error),rc);
        }
        //ftime(&end_time);
        //fprintf(stderr,"read %d bytes in %ld ms\n",sizeof(audio_buffer),((end_time.time*1000)+end_time.millitm)-((start_time.time*1000)+start_time.millitm));
    }


    // record the I/Q samples
    if(softrock_get_record()) {
        softrock_record_buffer(audio_buffer,sizeof(audio_buffer));
    }

    // de-interleave samples
    for(i=0;i<SAMPLES_PER_BUFFER;i++) {
        if(softrock_get_iq()) {
            left_samples[i]=audio_buffer[i*2];
            right_samples[i]=audio_buffer[(i*2)+1];
        } else {
            right_samples[i]=audio_buffer[i*2];
            left_samples[i]=audio_buffer[(i*2)+1];
        }
        if(timing) {
            sample_count++;
            if(sample_count==softrock_get_sample_rate()) {
                ftime(&end_time);
                fprintf(stderr,"%d samples in %ld ms\n",sample_count,((end_time.time*1000)+end_time.millitm)-((start_time.time*1000)+start_time.millitm));
                sample_count=0;
                ftime(&start_time);
            }
        }
    }

    return rc;
}
#endif
#ifdef PORTAUDIO
int softrock_write(float* left_samples,float* right_samples) {
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

int softrock_read(float* left_samples,float* right_samples) {
    int rc;
    int i;
    float audio_buffer[SAMPLES_PER_BUFFER*2];

    //fprintf(stderr,"read available=%ld\n",Pa_GetStreamReadAvailable(stream));
    rc=Pa_ReadStream(stream,audio_buffer,SAMPLES_PER_BUFFER);
    if(rc<0) {
        fprintf(stderr,"error reading audio_buffer %s (rc=%d)\n",Pa_GetErrorText(rc),rc);
    }

    // de-interleave samples
    if(softrock_get_iq()) {
        for(i=0;i<SAMPLES_PER_BUFFER;i++) {
            left_samples[i]=audio_buffer[i*2];
            right_samples[i]=audio_buffer[(i*2)+1];
//fprintf(stderr,"%d left=%f right=%f\n",i, left_samples[i],right_samples[i]);
        }
    } else {
        for(i=0;i<SAMPLES_PER_BUFFER;i++) {
            right_samples[i]=audio_buffer[i*2];
            left_samples[i]=audio_buffer[(i*2)+1];
//fprintf(stderr,"%d left=%f right=%f\n",i, left_samples[i],right_samples[i]);
        }
    }

    return rc;
}
#endif
#ifdef DIRECTAUDIO
int softrock_write(unsigned char* buffer,int buffer_size) {
    int rc;
    int bytes;

    rc = write(fd,buffer,buffer_size);
    if(rc!=buffer_size) {
        perror("error reading audio buffer");
        exit(1);
    }

    return rc;
}

int softrock_read(unsigned char* buffer,int buffer_size) {
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
