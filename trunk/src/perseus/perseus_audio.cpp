#include <stdio.h>
#include <stdlib.h>
#include "perseus_audio.h"





PerseusAudio :: PerseusAudio (int core_bandwidth) {
    //int arg;
    //int status;
    int rc;
    PaStreamParameters inputParameters;
    PaStreamParameters outputParameters;
    const PaStreamInfo *info;
    int devices;
    int i;
    const PaDeviceInfo* deviceInfo;


    fprintf(stderr,"PerseusAudio: portaudio\n");

    rc=Pa_Initialize();
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_Initialize failed: %s\n",Pa_GetErrorText(rc));
        throw "Pa_Initialize failed";
    }

    devices=Pa_GetDeviceCount();
    if(devices<0) {
        fprintf(stderr,"Px_GetDeviceCount failed: %s\n",Pa_GetErrorText(devices));
    } else {
        fprintf(stderr,"default input=%d output=%d devices=%d\n",
                       Pa_GetDefaultInputDevice(),
                       Pa_GetDefaultOutputDevice(),
                       devices);                                                
  
        for(i=0;i<devices;i++) {
            deviceInfo = Pa_GetDeviceInfo(i);
            fprintf(stderr,"%d - %s\n",i,deviceInfo->name);
            fprintf(stderr,"maxInputChannels: %d\n",deviceInfo->maxInputChannels);
            fprintf(stderr,"maxOututChannels: %d\n",deviceInfo->maxOutputChannels);
            fprintf(stderr,"defaultLowInputLatency: %f\n",deviceInfo->defaultLowInputLatency);
            fprintf(stderr,"defaultLowOutputLatency: %f\n",deviceInfo->defaultLowOutputLatency);
            fprintf(stderr,"defaultHighInputLatency: %f\n",deviceInfo->defaultHighInputLatency);
            fprintf(stderr,"defaultHighOutputLatency: %f\n",deviceInfo->defaultHighOutputLatency);
            fprintf(stderr,"defaultSampleRate: %f\n",deviceInfo->defaultSampleRate);
        }
    }

    inputParameters.device=Pa_GetDefaultInputDevice();
    inputParameters.channelCount=2;
    inputParameters.sampleFormat=paFloat32;
    inputParameters.suggestedLatency=Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo=NULL;

    outputParameters.device=Pa_GetDefaultOutputDevice();
    outputParameters.channelCount=2;
    outputParameters.sampleFormat=paFloat32;
    outputParameters.suggestedLatency=Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo=NULL;


    //
    // create sample rate subobjet
    //
    int sr_error;
    sr_state = src_new (
                        //SRC_SINC_BEST_QUALITY,  // NOT USABLE AT ALL on Atom 300 !!!!!!!
                        //SRC_SINC_MEDIUM_QUALITY,
                        SRC_SINC_FASTEST,
                        //SRC_ZERO_ORDER_HOLD,
                        //SRC_LINEAR,
                        2, &sr_error
                       ) ;

    if (sr_state == 0) { fprintf (stderr, "SR INIT ERROR: %s\n", src_strerror (sr_error)); throw src_strerror (sr_error); }

    //
    // compute sample rate
    //
    CORE_BANDWIDTH = core_bandwidth;

    deviceInfo = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
    fprintf (stderr, "Pa_GetDefaultInputDevice: defaultSampleRate: %f\n",deviceInfo->defaultSampleRate );
    SAMPLE_RATE =  deviceInfo->defaultSampleRate/4;
    fprintf (stderr,"input device=%d output device=%d\n",inputParameters.device,outputParameters.device);

    fprintf (stderr, "************* AUDIO SR: %f / %d = %f\n", SAMPLE_RATE, CORE_BANDWIDTH, (float)SAMPLE_RATE/(float)CORE_BANDWIDTH);
    rc=Pa_OpenStream (&stream,
                      &inputParameters,
                      &outputParameters,
                      (double)SAMPLE_RATE,
                      (unsigned long)SAMPLES_PER_BUFFER,
                      paNoFlag,
                      NULL,
                      NULL
    );
/*
    rc=Pa_OpenDefaultStream(&stream,CHANNELS,CHANNELS,paFloat32,SAMPLE_RATE,SAMPLES_PER_BUFFER,NULL, NULL);
*/
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_OpenStream failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }

    rc = Pa_StartStream(stream);
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_StartStream failed: %s\n",Pa_GetErrorText(rc));
        exit(1);
    }

    info = Pa_GetStreamInfo (stream);
    if(info!=NULL) {
        fprintf(stderr,"sample rate wanted=%f got=%f\n",SAMPLE_RATE,info->sampleRate);
    } else {
        fprintf(stderr,"Pa_GetStreamInfo returned NULL\n");
    }
}

PerseusAudio :: ~PerseusAudio () {

    fprintf (stderr, "********************* %s\n", __FUNCTION__);
    src_delete (sr_state) ;

    int rc=Pa_Terminate();
    if(rc!=paNoError) {
        fprintf(stderr,"Pa_Terminate failed: %s\n",Pa_GetErrorText(rc));
        throw "Pa_Terminate failed";
    }
}

int PerseusAudio :: write(float* left_samples,float* right_samples) {
    int rc;
    int i;
    float audio_buffer[SAMPLES_PER_BUFFER*2];

    // interleave samples
    for(i=0;i<SAMPLES_PER_BUFFER;i++) {
        audio_buffer[i*2]=right_samples[i];
        audio_buffer[(i*2)+1]=left_samples[i];
    }

    rc=Pa_WriteStream(stream,audio_buffer,SAMPLES_PER_BUFFER);
    if(rc!=0) {
        fprintf(stderr,"error writing audio_buffer %s (rc=%d)\n",Pa_GetErrorText(rc),rc);
    }

    return rc;
}


int PerseusAudio :: write_2 (float* left_samples, float* right_samples) {
    int rc;
    int i;
    float audio_buffer[SAMPLES_PER_BUFFER*2/DECIM_FACT];

    // interleave samples
    for(i=0;i<SAMPLES_PER_BUFFER;i++) {

        if ((i % DECIM_FACT) != 0) {
            audio_buffer[i/DECIM_FACT*2]=right_samples[i];
            audio_buffer[(i/DECIM_FACT*2)+1]=left_samples[i];
        }
    }

    rc = Pa_WriteStream (stream, audio_buffer, SAMPLES_PER_BUFFER/DECIM_FACT);
    if ( rc != 0 ) {
        fprintf(stderr,"error writing audio_buffer %s (rc=%d)\n", Pa_GetErrorText(rc), rc);
    }

    return rc;
}


int PerseusAudio :: write_3 (float* left_samples, float* right_samples) {
    int rc;
    int i;
    float audio_buffer[SAMPLES_PER_BUFFER*2/DECIM_FACT];

    // interleave samples
    for(i=0;i<SAMPLES_PER_BUFFER;i++) {

        if ((i % DECIM_FACT) == 0) {
            audio_buffer[i/DECIM_FACT*2]=right_samples[i];
            audio_buffer[(i/DECIM_FACT*2)+1]=left_samples[i];
        }
    }

    rc = Pa_WriteStream (stream, audio_buffer, SAMPLES_PER_BUFFER/DECIM_FACT);
    if ( rc != 0 ) {
        fprintf(stderr,"error writing audio_buffer %s (rc=%d)\n", Pa_GetErrorText(rc), rc);
    }

    return rc;
}

/*
typedef struct
      {   float  *data_in, *data_out ;

          long   input_frames, output_frames ;
          long   input_frames_used, output_frames_gen ;

          int    end_of_input ;

          double src_ratio ;
      } SRC_DATA ;

      data_in       : A pointer to the input data samples.
      input_frames  : The number of frames of data pointed to by data_in.
      data_out      : A pointer to the output data samples.
      output_frames : Maximum number of frames pointer to by data_out.
      src_ratio     : Equal to output_sample_rate / input_sample_rate.
      end_of_input  : Equal to 0 if more input data is available and 1 
                      otherwise.
*/
int PerseusAudio :: write_sr (float* left_samples, float* right_samples) {
    int rc;
    int i;
    float data_in [SAMPLES_PER_BUFFER*2];
    float data_out[SAMPLES_PER_BUFFER*2];
    SRC_DATA data;


    for (i=0;i < SAMPLES_PER_BUFFER; i++) {
        data_in [i*2]   = left_samples[i];
        data_in [i*2+1] = right_samples[i];
    }

    data.data_in = data_in;
    data.input_frames = SAMPLES_PER_BUFFER ;

    data.data_out = data_out;
    data.output_frames = SAMPLES_PER_BUFFER ;

    //data.src_ratio = (double) 1.0; //CORE_BANDWIDTH / 48000.0;
    //data.src_ratio = (double) CORE_BANDWIDTH / 48000.0;
    //data.src_ratio =  ((double) CORE_BANDWIDTH / (double)SAMPLE_RATE);
    data.src_ratio =  (SAMPLE_RATE / (double)CORE_BANDWIDTH);
    data.end_of_input = 0;


    rc = src_process (sr_state, &data) ;
    if (rc) {
        fprintf (stderr,"SRATE: error: %s (rc=%d)\n", src_strerror (rc), rc);
    } else {
        //if (data.input_frames_used != SAMPLES_PER_BUFFER  || data.output_frames_gen != SAMPLES_PER_BUFFER) 
        //   fprintf (stderr,"SRATE: %f used: %ld ge: %ld\n", data.src_ratio, data.input_frames_used, data.output_frames_gen);

        // copy the data output form samplerate
        // interleave samples
        float audio_buffer[SAMPLES_PER_BUFFER*2];

        for (i=0;i < data.output_frames_gen;i++) {
            audio_buffer[i*2]    = data.data_out[i*2];
            audio_buffer[i*2+1]  = data.data_out[i*2+1];
        }

        rc = Pa_WriteStream (stream, audio_buffer, data.output_frames_gen);
        if ( rc != 0 ) {
            fprintf (stderr,"PA: error writing audio_buffer %s (rc=%d)\n", Pa_GetErrorText(rc), rc);
        }
    }

    return rc;
}



