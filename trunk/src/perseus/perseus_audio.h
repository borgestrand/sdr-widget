#if !defined __PERSUES_AUDIO_H__
#define      __PERSUES_AUDIO_H__

#if 0
int perseus_audio_open    (int core_bandwidth) ;
int perseus_audio_close   (void) ;
int perseus_audio_write   (float* left_samples,float* right_samples) ;
int perseus_audio_write_2 (float* left_samples,float* right_samples) ;
int perseus_audio_write_3 (float* left_samples,float* right_samples) ;
#endif

#include <samplerate.h>
#include <portaudio.h>


class PerseusAudio  {
public:

    PerseusAudio (int core_bandwidth) ;                       
    ~PerseusAudio () ;                                     
    int write   (float* left_samples,float* right_samples) ; 
    int write_2 (float* left_samples,float* right_samples) ; 
    int write_3 (float* left_samples,float* right_samples) ; 
    int write_sr (float* left_samples, float* right_samples) ;

private:

    enum { CHANNELS = 2 };       /* 1 = mono 2 = stereo */
    enum { SAMPLES_PER_BUFFER = 1024 };

    int   DECIM_FACT;
    float SAMPLE_RATE;
    int   CORE_BANDWIDTH;

    PaStream* stream;

    SRC_STATE *sr_state;
    SRC_DATA  *sr_data;

};

#endif
