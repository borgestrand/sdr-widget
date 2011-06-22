/**
* @file sdr1000.c
* @brief sdr1000 audio implementation
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
#include <pthread.h>

#include "client.h"
#include "sdr1000.h"
#include "sdr1000io.h"
#include "receiver.h"
#include "util.h"

static pthread_t sdr1000_io_thread_id;

static int rx_frame=0;
static int tx_frame=0;
static int receivers=1;
static int current_receiver=0;

static int speed=0;
static int sample_rate=48000;

static int samples=0;

static int input_buffers;

static struct sockaddr_in client;
static int client_length;

static int iq_socket;
static struct sockaddr_in iq_address;
static int iq_address_length;

char device[80];
char input[80];
char output[80];

void* sdr1000_io_thread(void* arg);

#ifndef PORTAUDIO
void process_sdr1000_input_buffer(char* buffer);
#endif

int create_sdr1000_thread() {
    int rc;

    rc=sdr1000_init();
    if(rc<0) exit(1);

    // create a thread to read from the audio deice
    rc=pthread_create(&sdr1000_io_thread_id,NULL,sdr1000_io_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on sdr1000_io_thread: rc=%d\n", rc);
        exit(1);
    }

    return 0;
}

void sdr1000_set_device(char* d) {
fprintf(stderr,"sdr1000_set_device %s\n",d);
    strcpy(device,d);
}

char* sdr1000_get_device() {
fprintf(stderr,"sdr1000_get_device %s\n",device);
    return device;
}

void sdr1000_set_input(char* d) {
fprintf(stderr,"sdr1000_set_input %s\n",d);
    strcpy(input,d);
}

char* sdr1000_get_input() {
fprintf(stderr,"sdr1000_get_input %s\n",input);
    return input;
}

void sdr1000_set_output(char* d) {
fprintf(stderr,"sdr1000_set_output %s\n",d);
    strcpy(output,d);
}

char* sdr1000_get_output() {
fprintf(stderr,"sdr1000_get_output %s\n",output);
    return output;
}

void sdr1000_set_receivers(int r) {
    if(r>MAX_RECEIVERS) {
        fprintf(stderr,"MAX Receivers is 8!\n");
        exit(1);
    }
    receivers=r;
}

int sdr1000_get_receivers() {
    return receivers;
}

void sdr1000_set_sample_rate(int r) {
fprintf(stderr,"sdr1000_set_sample_rate %d\n",r);
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
}

int sdr1000_get_sample_rate() {
fprintf(stderr,"sdr1000_get_sample_rate %d\n",sample_rate);
    return sample_rate;
}

int sdr1000_init() {
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

    // open sdr1000 audio
    rc = sdr1000_open();
    if (rc != 0) {
        fprintf(stderr,"Cannot open sdr1000\n");
        return (-1);
    }

    for(i=0;i<receivers;i++) {
        receiver[i].frequency=7056000L;
        receiver[i].frequency_changed=1;
    }

fprintf(stderr,"server configured for %d receivers at %d\n",receivers,sample_rate);
    return rc;
}

void* sdr1000_io_thread(void* arg) {
#ifndef PORTAUDIO
    unsigned char input_buffer[BUFFER_SIZE*2]; // samples * 2 * 2
    int bytes;
#endif
    int rc;
    int i,j;

    while(1) {

#ifdef PORTAUDIO
        // read an input buffer (blocks until all bytes read)
        rc=sdr1000_read(receiver[current_receiver].input_buffer,&receiver[current_receiver].input_buffer[BUFFER_SIZE]);
        if(rc==0) {
            // process input buffer
            rx_frame++;
            input_buffers++;
            send_IQ_buffer(current_receiver);
        } else {
            fprintf(stderr,"sdr1000_read returned %d\n",rc);
        }
#else
        // read an input buffer (blocks until all bytes read)
        bytes=sdr1000_read(input_buffer,sizeof(input_buffer));
        if (bytes < 0) {
            fprintf(stderr,"sdr1000_io_thread: read failed %d\n",bytes);
        } else if (bytes != sizeof(input_buffer)) {
            fprintf(stderr,"sfoftrock_io_thread: only read %d bytes\n",bytes);
        } else {
            // process input buffer
            rx_frame++;
            process_sdr1000_input_buffer(input_buffer);
        }
        input_buffers++;
#endif
        current_receiver++;

        if(current_receiver==receivers) {
            current_receiver=0;
        }
    }
}

#ifndef PORTAUDIO
void process_sdr1000_input_buffer(char* buffer) {
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

#ifdef PORTAUDIO
void process_sdr1000_output_buffer(float* left_output_buffer,float* right_output_buffer) {

    sdr1000_write(left_output_buffer,right_output_buffer);
    
}
#else
void process_sdr1000_output_buffer(float* left_output_buffer,float* right_output_buffer) {
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

    sdr1000_write(output_buffer,sizeof(output_buffer));
}


#endif
