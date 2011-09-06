/**
* @file softrock.c
* @brief Softrock audio implementation
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include "client.h"
#include "softrock.h"
#include "softrockio.h"
#include "receiver.h"
#include "util.h"

static pthread_t softrock_io_thread_id;

static int rx_frame=0;
//static int tx_frame=0;
static int receivers=1;
static int current_receiver=0;

static int speed=0;
static int sample_rate=48000;

//static int samples=0;

static int input_buffers;

//static struct sockaddr_in client;
//static int client_length;

static int iq_socket;
static struct sockaddr_in iq_address;
static int iq_address_length;

char device[80];
char input[80];
char output[80];

static int iq=1;


static char filename[256];
static int record=0;
static int playback=0;
static int playback_sleep=0;
static FILE* recording;

void* softrock_io_thread(void* arg);

#if (defined PULSEAUDIO || defined DIRECTAUDIO)
void process_softrock_input_buffer(char* buffer);
#endif

#ifdef JACKAUDIO
/* global jack variables. */
jack_client_t *softrock_client;
jack_port_t *audio_input_port_left[MAX_RECEIVERS], *audio_input_port_right[MAX_RECEIVERS];
#endif

int softrock_init(void);


int create_softrock_thread() {
#ifndef JACKAUDIO
    int rc;
#endif
    softrock_init();
#ifndef JACKAUDIO //(Using callback instead)
    // create a thread to read from the audio deice
    rc=pthread_create(&softrock_io_thread_id,NULL,softrock_io_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on softrock_io_thread: rc=%d\n", rc);
        exit(1);
    }
	return 0;
#endif
#ifdef JACKAUDIO //(Using callback)
	if (init_jack_audio() != 0) {
		fprintf(stderr, "There was a problem initializing Jack Audio.\n");
  	return 1;
	}
	else
	{
		return 0;
	}
#endif			
}


void softrock_set_device(char* d) {
fprintf(stderr,"softrock_set_device %s\n",d);
    strcpy(device,d);
}

char* softrock_get_device() {
fprintf(stderr,"softrock_get_device %s\n",device);
    return device;
}

void softrock_set_input(char* d) {
fprintf(stderr,"softrock_set_input %s\n",d);
    strcpy(input,d);
}

char* softrock_get_input() {
fprintf(stderr,"softrock_get_input %s\n",input);
    return input;
}

void softrock_set_output(char* d) {
fprintf(stderr,"softrock_set_output %s\n",d);
    strcpy(output,d);
}

char* softrock_get_output() {
fprintf(stderr,"softrock_get_output %s\n",output);
    return output;
}

void softrock_set_receivers(int r) {
    if(r>MAX_RECEIVERS) {
        fprintf(stderr,"MAX Receivers is 8!\n");
        exit(1);
    }
    receivers=r;
}

int softrock_get_receivers() {
    return receivers;
}

void softrock_set_sample_rate(int r) {
fprintf(stderr,"softrock_set_sample_rate %d\n",r);
    switch(r) {
        case 48000:
            sample_rate=r;
            speed=0;
            break;
        case 96000:
            sample_rate=r;
            speed=1;
            break;
        case 192000:
            sample_rate=r;
            speed=2;
            break;
        default:
            fprintf(stderr,"Invalid sample rate (48000,96000,192000)!\n");
            exit(1);
            break;
    }
    playback_sleep=(int)(1024.0/(float)sample_rate*1000000.0);
    fprintf(stderr,"sample_rate=%d playback_sleep=%d\n",sample_rate,playback_sleep);

}

int softrock_get_sample_rate() {
    return sample_rate;
}

void softrock_set_iq(int s) {
    iq=s;
}

int softrock_get_iq() {
    return iq;
}

int softrock_get_record() {
    return record;
}

int softrock_get_playback() {
    return playback;
}

void softrock_set_record(char* f) {
    strcpy(filename,f);
    record=1;
    playback=0;
}

void softrock_set_playback(char* f) {
    strcpy(filename,f);
    record=0;
    playback=1;
}

int softrock_init(void) {
    int rc;
    int i;

    iq_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(iq_socket<0) {
        perror("create socket failed for iq samples");
        exit(1);
    }

    iq_address_length=sizeof(iq_address);
    memset(&iq_address,0,iq_address_length);
    iq_address.sin_family=AF_INET;
    iq_address.sin_addr.s_addr=htonl(INADDR_ANY);
    iq_address.sin_port=htons(0);

    if(bind(iq_socket,(struct sockaddr*)&iq_address,iq_address_length)<0) {
        perror("bind socket failed for iq socket");
        exit(1);
    }

    if(record) {
        recording=fopen(filename,"w");
    } else if(playback) {
        recording=fopen(filename,"r");
				fprintf(stderr,"opening %s\n",filename);
    }

    // open softrock audio
    rc = softrock_open();
    if (rc != 0) {
        fprintf(stderr,"Cannot open softrock\n");
        return (-1);
    }

    for(i=0;i<receivers;i++) {
        receiver[i].frequency=7056000L;
        receiver[i].frequency_changed=1;
    }

		fprintf(stderr,"server configured for %d receivers at %d\n",receivers,sample_rate);
    return rc;
}

void softrock_record_buffer(char* buffer,int length) {
    int bytes;

    if(record) {
        bytes=fwrite(buffer,sizeof(char),length,recording);
    }
}

void softrock_playback_buffer(char* buffer,int length) {
    int bytes;

    if(playback) {
        usleep(playback_sleep);
        bytes=fread(buffer,sizeof(char),length,recording);
        if(bytes<=0) {
            // assumes eof
            fclose(recording);
            recording=fopen(filename,"r");
						fprintf(stderr,"playback: re-opening %s\n",filename);
            bytes=fread(buffer,sizeof(char),length,recording);
        } else {
					//fprintf(stderr,"playback: read %d bytes\n",bytes);
        }
    }
}

#ifndef JACKAUDIO
void* softrock_io_thread(void* arg) {
#if (defined PULSEAUDIO || defined PORTAUDIO)
    int rc;
#else    
    unsigned char input_buffer[BUFFER_SIZE*2]; // samples * 2 * 2
    int bytes;
#endif
 
    while(1) {

#ifdef PULSEAUDIO
        // read an input buffer (blocks until all bytes read)
        rc=softrock_read(receiver[current_receiver].input_buffer,&receiver[current_receiver].input_buffer[BUFFER_SIZE]);
        if(rc==0) {
            // process input buffer
            rx_frame++;
            input_buffers++;
            send_IQ_buffer(current_receiver);
        } else {
            fprintf(stderr,"softrock_read returned %d\n",rc);
        }
#endif
#ifdef PORTAUDIO
        // read an input buffer (blocks until all bytes read)
        rc=softrock_read(receiver[current_receiver].input_buffer,&receiver[current_receiver].input_buffer[BUFFER_SIZE]);
        if(rc==0) {
            // process input buffer
            rx_frame++;
            input_buffers++;
            send_IQ_buffer(current_receiver);
        } else {
            fprintf(stderr,"softrock_read returned %d\n",rc);
        }
#endif
#ifdef DIRECTAUDIO
        // read an input buffer (blocks until all bytes read)
        bytes=softrock_read(input_buffer,sizeof(input_buffer));
        if (bytes < 0) {
            fprintf(stderr,"softrock_io_thread: read failed %d\n",bytes);
        } else if (bytes != sizeof(input_buffer)) {
            fprintf(stderr,"sfoftrock_io_thread: only read %d bytes\n",bytes);
        } else {
            // process input buffer
            rx_frame++;
            process_softrock_input_buffer(input_buffer);
        }
        input_buffers++;
#endif
        current_receiver++;

        if(current_receiver==receivers) {
            current_receiver=0;
        }
    }
}
#endif

#ifdef DIRECTAUDIO
void process_softrock_input_buffer(char* buffer) {
    int b=0;
    int r;
    short left_sample,right_sample;
    //int left_sample,right_sample;
    float left_sample_float,right_sample_float;
    int rc;

        // extract the samples
    while(b<(BUFFER_SIZE*2*2)) {
        // extract each of the receivers
        for(r=0;r<receivers;r++) {
					//fprintf(stderr,"%d: %02X%02X %02X%02X\n",samples,buffer[b]&0xFF,buffer[b+1]&0xFF,buffer[b+2]&0xFF,buffer[b+3]&0xFF);
            left_sample   = (int)((unsigned char)buffer[b++]);
            left_sample  |= (int)((signed char)buffer[b++])<<8;
            //left_sample  += (int)((unsigned char)buffer[b++])<<8;
            //left_sample  += (int)((signed char)buffer[b++])<<16;
            right_sample  = (int)((unsigned char)buffer[b++]);
            right_sample |= (int)((signed char)buffer[b++])<<8;
            //right_sample += (int)((unsigned char)buffer[b++])<<8;
            //right_sample += (int)((signed char)buffer[b++])<<16;
            left_sample_float=(float)left_sample/32767.0; // 16 bit sample
            right_sample_float=(float)right_sample/32767.0; // 16 bit sample
/*
            left_sample_float=(float)left_sample/8388607.0; // 24 bit sample
            right_sample_float=(float)right_sample/8388607.0; // 24 bit sample
*/
            receiver[r].input_buffer[samples]=left_sample_float;
            receiver[r].input_buffer[samples+BUFFER_SIZE]=right_sample_float;

						//fprintf(stderr,"%d: %d %d\n",samples,left_sample,right_sample);
						//fprintf(stderr,"%d: %f %f\n",samples,left_sample_float,right_sample_float);
        }
        samples++;

        // when we have enough samples send them to the clients
        if(samples==BUFFER_SIZE) {
            // send I/Q data to clients
            for(r=0;r<receivers;r++) {
                send_IQ_buffer(r);
            }
            samples=0;
        }
    }

}
#endif

#ifdef PULSEAUDIO
void process_softrock_output_buffer(float* left_output_buffer,float* right_output_buffer) {

    softrock_write(left_output_buffer,right_output_buffer);
    
}
#endif
#ifdef PORTAUDIO
void process_softrock_output_buffer(float* left_output_buffer,float* right_output_buffer) {

    softrock_write(left_output_buffer,right_output_buffer);
    
}
#endif
#ifdef DIRECTAUDIO
void process_softrock_output_buffer(float* left_output_buffer,float* right_output_buffer) {
    int i;
    unsigned char output_buffer[BUFFER_SIZE*2*2];
    int left_sample,right_sample;
    int b=0;

    for(i=0;i<BUFFER_SIZE;i++) {
        left_sample=(int)(left_output_buffer[i]*32767.0F);
        right_sample=(int)(right_output_buffer[i]*32767.0F);
/*
        left_sample=(int)(left_output_buffer[i]*8388607.0F);
        right_sample=(int)(right_output_buffer[i]*8388607.0F);
*/
        output_buffer[b++]=left_sample&0xFF;
        output_buffer[b++]=(left_sample>>8)&0xFF;
        //output_buffer[b++]=(left_sample>>16)&0xFF;
        output_buffer[b++]=right_sample&0xFF;
        output_buffer[b++]=(right_sample>>8)&0xFF;
        //output_buffer[b++]=(right_sample>>16)&0xFF;
    }

    softrock_write(output_buffer,sizeof(output_buffer));
}


#endif

#ifdef JACKAUDIO

int init_jack_audio()
{
	int verbose_flag = 1, error, r;

	const char * capture_port_name[2*MAX_RECEIVERS] = {"system:capture_1","system:capture_2",
		"system:capture_3","system:capture_4","system:capture_5","system:capture_6",
		"system:capture_7","system:capture_8"};
	const char * softrock_port_name_left[MAX_RECEIVERS] = {"Softrock Port_1_left",
		"Softrock Port_2_left","Softrock Port_3_left","Softrock Port_4_left",};
	const char * softrock_port_name_right[MAX_RECEIVERS] = {"Softrock Port_1_right",
		"Softrock Port_2_right","Softrock Port_3_right","Softrock Port_4_right"};
	
	//Create a new jack client, then make sure everything went ok.
	softrock_client = jack_client_open("Softrock",(jack_options_t)(!JackServerName),NULL);
	if (softrock_client == 0) {
		fprintf(stderr,"Cannot connect to the jackd as a client.\n");
		jack_cleanup();
		return 1;
	}
	
	/* Set up Jack */
	//Set up the jack shutdown routine in case we want to do something special on shutdown of jack.
	jack_on_shutdown (softrock_client, jack_shutdown, 0);

	//Check to make sure the buffer size isn't too big.
	if(jack_get_buffer_size (softrock_client)	> BUFFER_SIZE) {
		fprintf(stderr,"Jack Buffers is too large.  Either recompile with a larger BUFFER_SIZE,\n",
		        "or start Jack with a buffer size of %d.\n",BUFFER_SIZE);
		jack_cleanup ();
		return 1;
	}

	//Create and register new audio input ports.
	for(r=0;r<receivers;r++) {		
		audio_input_port_left[r] = jack_port_register(softrock_client, softrock_port_name_left[r], 
		                                           JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
		if (audio_input_port_left[r] == NULL) {
			fprintf(stderr, "Error: jack_port_register returned NULL for %s.\n",softrock_port_name_left[r]);
			jack_cleanup();
			return 1;
		}

		audio_input_port_right[r] = jack_port_register(softrock_client, softrock_port_name_right[r], 
		                                            JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
		if (audio_input_port_right[r] == NULL) {
			fprintf(stderr, "Error: jack_port_register returned NULL for %s.\n",softrock_port_name_right[r]);
			jack_cleanup();
			return 1;
		}
	}
	
	//Tell the jackd server what function call when it wants more audio data.
	if((error = jack_set_process_callback(softrock_client, process,0)) != 0) { 
		fprintf(stderr, "Jack could not set the callback, (error %i).\n", error);
		jack_cleanup();
		return 1;
	}

	//Tell jack it's ok to start asking us for audio data.
	if((error = jack_activate(softrock_client)) != 0) {
		fprintf(stderr, "Jack could not activate the client (error %i).\n", error);
		jack_cleanup();
		return 1;
	}
	else if(verbose_flag) fprintf(stderr,"Activated client.\n");

	//Connect the ports.
	for(r=0;r<receivers;r++) {
		if (jack_connect (softrock_client,capture_port_name[2*r], jack_port_name (audio_input_port_left[r]))) {
			fprintf (stderr, "cannot connect port: %s\n",capture_port_name[2*r]);
		}
		else if(verbose_flag) fprintf(stderr, "Connected to port: %s\n",capture_port_name[2*r]);
		if (jack_connect (softrock_client,capture_port_name[2*r+1], jack_port_name (audio_input_port_right[r]))) {
			fprintf (stderr, "cannot connect port:  %s\n",capture_port_name[2*r+1]);
		}
		else if(verbose_flag) fprintf(stderr, "Connected to port:   %s\n",capture_port_name[2*r+1]);
	}
	return 0;
}

/* This is the function that is called when and if jackd shuts down. */
void jack_shutdown (void *arg)
{
	fprintf (stderr, "JACK shutdown\n");
	jack_cleanup();
	abort();
}

/* Close things opened. */
void jack_cleanup(void) {
	if(softrock_client != NULL) {
		jack_deactivate(softrock_client);	
		jack_client_close(softrock_client);
	}
}


/* This is the callback process that gets data from jack.*/	
int process(jack_nframes_t number_of_frames, void* arg)
{
	// Start out with current_receiver = 0 (one receiver) fix later.
	jack_nframes_t i;
	int r;
	jack_default_audio_sample_t *sample_buffer_left[MAX_RECEIVERS];
	jack_default_audio_sample_t *sample_buffer_right[MAX_RECEIVERS];

	rx_frame++;
	input_buffers++;

	float *left_samples, *right_samples;

	for ( r = 0; r < receivers; r++ ) {
		sample_buffer_left[r] = 
				(jack_default_audio_sample_t *) jack_port_get_buffer(audio_input_port_left[r], number_of_frames);
		sample_buffer_right[r] = 
				(jack_default_audio_sample_t *) jack_port_get_buffer(audio_input_port_right[r], number_of_frames);
		left_samples = &receiver[r].input_buffer[0];
		right_samples = &receiver[r].input_buffer[BUFFER_SIZE];
		if(softrock_get_iq()) {
			for(i=0;i<number_of_frames;i++) {
				left_samples[i]=(float)sample_buffer_left[r][i];
				right_samples[i]=(float)sample_buffer_right[r][i];
				//fprintf(stderr,"%d left=%f right=%f\n",i, left_samples[i],right_samples[i]);
			}
		} else {
			for(i=0;i<number_of_frames;i++) {
				right_samples[i]=(float)sample_buffer_left[r][i];
				left_samples[i]=(float)sample_buffer_right[r][i];
				//fprintf(stderr,"%d left=%f right=%f\n",i, left_samples[i],right_samples[i]);
			}
		}
		send_IQ_buffer(r);
	}

	return 0;
}

#endif

