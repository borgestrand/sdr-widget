#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <poll.h>
#include <alsa/asoundlib.h>
#include <semaphore.h>


static sem_t audio_sem;
static pthread_t *write_local_audio_thread_id;
void write_local_audio_thread(void* arg);
	      
snd_pcm_t *playback_handle;

#define CHANNELS 2
#define SAMPLE_RATE 48000
#define SAMPLE_PER_BUFFER 2048
#define AUDIO_BUFFER_SIZE (SAMPLE_PER_BUFFER * sizeof(short)*CHANNELS*2)
#define ALSA_BUF_SIZE 2048
#define ALSA_PERIOD_SIZE 128

static char local_audio_device[32];
static int insert=0;
static int out_index = 0;
static unsigned char audio_buffer[AUDIO_BUFFER_SIZE];
static unsigned char buf[ALSA_PERIOD_SIZE*CHANNELS*sizeof(short)];
// static snd_async_handler_t *pcm_callback;
static snd_pcm_uframes_t buffer_size = ALSA_BUF_SIZE;
static snd_pcm_uframes_t period_size = ALSA_PERIOD_SIZE;
static snd_pcm_sframes_t avail;
static unsigned int sample_rate = SAMPLE_RATE;




void set_local_audio_device(char * device) {
    strcpy(local_audio_device,device);
}

int open_local_audio() {

	fprintf(stderr,"open_local_audio: alsa\n");

    	/* open sound device */
	
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_sw_params_t *sw_params;

	int err;
	int rc;

	if ((err = snd_pcm_open (&playback_handle, local_audio_device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n", 
			 local_audio_device,
			 snd_strerror (err));
		exit (1);
	}
	   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
			 
	if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_BE)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &sample_rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, CHANNELS)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	if ((err = snd_pcm_hw_params_set_buffer_size_near (playback_handle, hw_params, &buffer_size)) < 0) {
		fprintf (stderr, "cannot set buffer size (%s)\n",
			 snd_strerror (err));
		exit (1);
	}


	if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

	snd_pcm_hw_params_free (hw_params);

	/* tell ALSA to wake us up whenever SAMPLES_PER_FRAME or more frames
	   of playback data can be delivered. Also, tell
	   ALSA that we'll start the device ourselves.
	*/

	if ((err = snd_pcm_sw_params_malloc (&sw_params)) < 0) {
		fprintf (stderr, "cannot allocate software parameters structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
	if ((err = snd_pcm_sw_params_current (playback_handle, sw_params)) < 0) {
		fprintf (stderr, "cannot initialize software parameters structure (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
	if ((err = snd_pcm_sw_params_set_avail_min (playback_handle, sw_params, buffer_size)) < 0) {
		fprintf (stderr, "cannot set minimum available count (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
	if ((err = snd_pcm_sw_params_set_start_threshold (playback_handle, sw_params, 0U)) < 0) {
		fprintf (stderr, "cannot set start mode (%s)\n",
			 snd_strerror (err));
		exit (1);
	}
	if ((err = snd_pcm_sw_params (playback_handle, sw_params)) < 0) {
		fprintf (stderr, "cannot set software parameters (%s)\n",
			 snd_strerror (err));
		exit (1);
	}


	if ((err = snd_pcm_prepare (playback_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
			 snd_strerror (err));
		exit (1);
	}

// write something first to prevent xrun

	if ((err = snd_pcm_writei (playback_handle, buf, 2*period_size)) < 0) {
		fprintf (stderr, "write failed (%s)\n", snd_strerror (err));
	        exit (1);
	}

/*
	if ((err = snd_async_add_pcm_handler(&pcm_callback, playback_handle, playback_callback, NULL)) < 0) {
		fprintf (stderr, "add pcm handler failed (%s)\n", snd_strerror (err));
		exit (1);
	}
*/	

	fprintf(stderr, "open_local_audio: alsa .... success!!!\n");

	sem_init (&audio_sem, 0, 0);
	rc=pthread_create(&write_local_audio_thread_id,NULL, write_local_audio_thread,NULL);
	if(rc!=0) {
		fprintf(stderr,"pthread_create failed on write_local_audio_thread: rc=%d\n",rc);
	    }


    return 0;

}

int close_local_audio() {
//	snd_async_del_handler (pcm_callback);
	snd_pcm_close (playback_handle);
	return 0;
}

int write_local_audio(float* left_samples,float* right_samples,int samples,int increment) {
    int i;
    short sample;

    for(i=0;i<samples;i+=increment) {
        sample=(short)(left_samples[i]*32767.0F);
        audio_buffer[insert++]=sample>>8;
        audio_buffer[insert++]=sample;
        sample=(short)(right_samples[i]*32767.0F);
        audio_buffer[insert++]=sample>>8;
        audio_buffer[insert++]=sample;
    }
    sem_post(&audio_sem);
    if (insert >= AUDIO_BUFFER_SIZE) insert = 0;

    return 0;
}

void write_local_audio_thread(void* arg) {
    int i;

    while(1) {
        	sem_wait(&audio_sem);
		avail = snd_pcm_avail_update(playback_handle);
 		while (avail >= period_size) {
                        for (i = 0; i < (ALSA_PERIOD_SIZE * CHANNELS * sizeof(short) ); i++){
				buf[i] = audio_buffer[out_index++];
				if (out_index >= AUDIO_BUFFER_SIZE) out_index = 0;
			}
     			snd_pcm_writei(playback_handle, buf, period_size);
    			avail = snd_pcm_avail_update(playback_handle);
     		}
	}
}




