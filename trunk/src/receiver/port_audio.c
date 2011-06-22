#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <portaudio.h>
#include <pthread.h>

#include "port_audio.h"

#define CHANNELS 2
#define SAMPLE_RATE 48000
#define SAMPLES_PER_BUFFER 2048
#define AUDIO_BUFFER_SIZE (SAMPLES_PER_BUFFER*CHANNELS*2)

static PaStream *stream;

static int insert=0;
static int out_index = 0;
static float audio_buffer[AUDIO_BUFFER_SIZE];
static sem_t audio_sem;
static int audio_device=0;

static int write_port_audio_thread_id;

typedef struct {
       float left_phase;
       float right_phase;
     } audio_out_data_t;

static audio_out_data_t data;

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/ 
static int write_callback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    /* Cast data passed through stream to our structure. */
    audio_out_data_t *data = (audio_out_data_t*)userData; 
    float *out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    for( i=0; i<framesPerBuffer; i++ )
    {
/*      
        // test code to generate saw tooth sound locally instead of reading from audio_buffer
   
        *out++ = data->left_phase;  //left
        *out++ = data->right_phase; //right
        //Generate simple sawtooth phaser that ranges between -1.0 and 1.0.
        data->left_phase += 0.01f;
        // When signal reaches top, drop back down.
        if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
        // higher pitch so we can distinguish left and right.
        data->right_phase += 0.03f;
        if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
*/
        *out++ = audio_buffer[out_index++];
        *out++ = audio_buffer[out_index++];
        if (out_index >= AUDIO_BUFFER_SIZE) out_index = 0;

    }
    return 0;
}

static void write_port_audio_thread(void* arg) {
    int rc;

    while(1) {
        sem_wait(&audio_sem);

/*
        rc=Pa_WriteStream(stream,audio_buffer,SAMPLES_PER_BUFFER);
        if(rc!=0) {
            fprintf(stderr,"error writing first audio_buffer %s (rc=%d)\n",Pa_GetErrorText(rc),rc);
        }
*/
        sem_wait(&audio_sem);
/*
        rc=Pa_WriteStream(stream,&audio_buffer[SAMPLES_PER_BUFFER],SAMPLES_PER_BUFFER);
        if(rc!=0) {
            fprintf(stderr,"error writing second audio_buffer %s (rc=%d)\n",Pa_GetErrorText(rc),rc);
        }
*/
    }

}

void set_port_audio_device(int device) {
    audio_device=device;
}

void open_port_audio() {
    int devices;
    int i;
    PaDeviceInfo* deviceInfo;
    PaStreamParameters outputParameters;

    fprintf(stderr,"open_port_audio\n");

    int rc=Pa_Initialize();
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
        }
    }

    
    outputParameters.device=audio_device;
    outputParameters.channelCount=2;
    outputParameters.sampleFormat=paFloat32;
    outputParameters.suggestedLatency=Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo=NULL;

    rc=Pa_OpenStream(&stream,NULL,&outputParameters,SAMPLE_RATE,SAMPLES_PER_BUFFER,paNoFlag, write_callback,&data);
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_OpenStream failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }

/*
    rc=Pa_OpenDefaultStream(&stream,0,CHANNELS,paFloat32,SAMPLE_RATE,SAMPLES_PER_BUFFER,
                            NULL, NULL);
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_OpenDefaultStream failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }
*/
    sem_init (&audio_sem, 0, 0);
    rc=pthread_create(&write_port_audio_thread_id,NULL,write_port_audio_thread,NULL);
    if(rc!=0) {
        fprintf(stderr,"pthread_create failed on write_port_audio_thread: rc=%d\n",rc);
    }

    rc=Pa_StartStream(stream);
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_StartStream failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }

}

void close_port_audio() {
    int rc=Pa_Terminate();
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_Terminate failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }
}

void write_port_audio(float* left_samples,float* right_samples,int samples,int increment) {
    int i;

    for(i=0;i<samples;i+=increment) {
        audio_buffer[insert++]=left_samples[i];
        audio_buffer[insert++]=right_samples[i];
        if(insert==SAMPLES_PER_BUFFER*2) {
            sem_post(&audio_sem);
        } else if(insert==(SAMPLES_PER_BUFFER*4)) {
            insert=0;
            sem_post(&audio_sem);
        }
    }

}
