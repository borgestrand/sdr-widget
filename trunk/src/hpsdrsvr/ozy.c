/**
* @file ozy.c
* @brief Ozy protocol implementation
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
#include <sys/timeb.h>

#include "ozy_ringbuffer.h"
#include "client.h"
#include "ozyio.h"
#include "bandscope.h"
#include "receiver.h"
#include "util.h"

#define OZY_BUFFERS 1
#define OZY_BUFFER_SIZE 512

#define SYNC 0x7F

// ozy command and control
#define MOX_DISABLED    0x00
#define MOX_ENABLED     0x01

#define MIC_SOURCE_JANUS 0x00
#define MIC_SOURCE_PENELOPE 0x80
#define CONFIG_NONE     0x00
#define CONFIG_PENELOPE 0x20
#define CONFIG_MERCURY  0x40
#define CONFIG_BOTH     0x60
#define PENELOPE_122_88MHZ_SOURCE 0x00
#define MERCURY_122_88MHZ_SOURCE  0x10
#define ATLAS_10MHZ_SOURCE        0x00
#define PENELOPE_10MHZ_SOURCE     0x04
#define MERCURY_10MHZ_SOURCE      0x08
#define SPEED_48KHZ               0x00
#define SPEED_96KHZ               0x01
#define SPEED_192KHZ              0x02

#define MODE_CLASS_E              0x01
#define MODE_OTHERS               0x00

#define ALEX_ATTENUATION_0DB      0x00
#define ALEX_ATTENUATION_10DB     0x01
#define ALEX_ATTENUATION_20DB     0x02
#define ALEX_ATTENUATION_30DB     0x03
#define LT2208_GAIN_OFF           0x00
#define LT2208_GAIN_ON            0x04
#define LT2208_DITHER_OFF         0x00
#define LT2208_DITHER_ON          0x08
#define LT2208_RANDOM_OFF         0x00
#define LT2208_RANDOM_ON          0x10

#define DUPLEX                    0x04

static pthread_t ep6_ep2_io_thread_id;
static pthread_t ep4_io_thread_id;

static int configure=2;
static int rx_frame=0;
static int tx_frame=0;
static int receivers=2;
static int current_receiver=0;

static int speed=1;
static int sample_rate=96000;
static int output_sample_increment=2;

static struct timeb start_time;
static struct timeb end_time;
static int sample_count=0;

static unsigned char control_in[5]={0x00,0x00,0x00,0x00,0x00};
static unsigned char control_out[5]={0x00,0x00,0x00,0x00,0x00};

static int ptt=0;
static int dot=0;
static int dash=0;
static int lt2208ADCOverflow=0;

static char ozy_firmware_version[9];
static int mercury_software_version=0;
static int penelope_software_version=0;
static int ozy_software_version=0;

static int forward_power=0;

static int samples=0;

static float mic_gain=0.26F;
static float mic_left_buffer[BUFFER_SIZE];
static float mic_right_buffer[BUFFER_SIZE];

static struct sockaddr_in client;
static int client_length;

static int iq_socket;
static struct sockaddr_in iq_address;
static int iq_address_length;

void* ozy_ep6_ep2_io_thread(void* arg);
void* ozy_ep4_io_thread(void* arg);

void process_ozy_input_buffer(char* buffer);
void process_bandscope_buffer(char* buffer);

int create_ozy_thread() {
    int rc;

    ozy_init();

    ftime(&start_time);

    // create a thread to read/write to EP6/EP2
    rc=pthread_create(&ep6_ep2_io_thread_id,NULL,ozy_ep6_ep2_io_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on ozy_ep6_io_thread: rc=%d\n", rc);
        exit(1);
    }

/*
    // create a thread to read from EP4
    rc=pthread_create(&ep4_io_thread_id,NULL,ozy_ep4_io_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on ozy_ep4_io_thread: rc=%d\n", rc);
        exit(1);
    }
*/
    return 0;
}

void ozy_set_receivers(int r) {
    if(r>MAX_RECEIVERS) {
        fprintf(stderr,"MAX Receivers is 8!\n");
        exit(1);
    }
    receivers=r;
}

int ozy_get_receivers() {
    return receivers;
}

void ozy_set_sample_rate(int r) {
    switch(r) {
        case 48000:
            sample_rate=r;
            speed=0;
            output_sample_increment = 1;
            break;
        case 96000:
            sample_rate=r;
            speed=1;
            output_sample_increment = 2;
            break;
        case 192000:
            sample_rate=r;
            speed=2;
            output_sample_increment = 4;
            break;
        default:
            fprintf(stderr,"Invalid sample rate (48000,96000,192000)!\n");
            exit(1);
            break;
    }
}

int ozy_get_sample_rate() {
    return sample_rate;
}

int ozy_init() {
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

    // setup defaults
    control_out[0] = MOX_DISABLED;
    control_out[1] = CONFIG_MERCURY | MERCURY_122_88MHZ_SOURCE | MERCURY_10MHZ_SOURCE | speed | MIC_SOURCE_PENELOPE;
    control_out[2] = MODE_OTHERS;
    control_out[3] = ALEX_ATTENUATION_0DB | LT2208_GAIN_OFF | LT2208_DITHER_ON | LT2208_RANDOM_ON;
    control_out[4] = DUPLEX | ((receivers-1)<<3);

    // open ozy
    rc = ozy_open();
    if (rc != 0) {
        fprintf(stderr,"Cannot locate Ozy\n");
        return (-1);
    }

    rc=ozy_get_firmware_string(ozy_firmware_version,8);
    if(rc!=0) {
        fprintf(stderr,"Failed to get Ozy Firmware Version - Have you run initozy yet?\n");
        ozy_close();
        return (-2);
    }

    fprintf(stderr,"Ozy FX2 version: %s\n",ozy_firmware_version);

    for(i=0;i<receivers;i++) {
        receiver[i].frequency=7056000L;
        receiver[i].frequency_changed=1;
    }

    create_ozy_ringbuffer(68*512);

fprintf(stderr,"server configured for %d receivers at %d\n",receivers,sample_rate);
    return rc;
}

void* ozy_ep6_ep2_io_thread(void* arg) {
    unsigned char input_buffer[OZY_BUFFER_SIZE*OZY_BUFFERS];
    unsigned char output_buffer[OZY_BUFFER_SIZE];
    int bytes;
    int i,j;
    int valid_output;
    int input_buffers;
    int force_output;

    input_buffers=0;
    force_output=0;

    while(1) {

        // read an input buffer (blocks until all bytes read)
        bytes=ozy_read(0x86,input_buffer,OZY_BUFFER_SIZE*OZY_BUFFERS);
        if (bytes < 0) {
            fprintf(stderr,"ozy_ep6_ep2_io_thread: OzyBulkRead read failed %d\n",bytes);
        } else if (bytes != OZY_BUFFER_SIZE*OZY_BUFFERS) {
            fprintf(stderr,"ozy_ep6_ep2_io_thread: OzyBulkRead only read %d bytes\n",bytes);
        } else {
            // process input buffer
            rx_frame++;

if(rx_frame<10) {
    dump_ozy_buffer("received from Ozy:",rx_frame,input_buffer);
}
            process_ozy_input_buffer(input_buffer);
        }
        input_buffers++;

        // see if we have enough to send a buffer
        // force a write if we are just starting up
        if(configure>0 || ((rx_frame%output_sample_increment)==0)) {
/*
        if(configure>0 || receiver[current_receiver].frequency_changed || ozy_ringbuffer_entries(ozy_output_buffer)>=(OZY_BUFFER_SIZE-8)) {
*/

            input_buffers=0;

            output_buffer[0]=SYNC;
            output_buffer[1]=SYNC;
            output_buffer[2]=SYNC;

            if(configure>0) {
                configure--;
                output_buffer[3]=control_out[0];
                output_buffer[4]=control_out[1];
                output_buffer[5]=control_out[2];
                output_buffer[6]=control_out[3];
                output_buffer[7]=control_out[4];
            } else if(receiver[current_receiver].frequency_changed) {
                output_buffer[3]=control_out[0]|((current_receiver+2)<<1);
                output_buffer[4]=receiver[current_receiver].frequency>>24;
                output_buffer[5]=receiver[current_receiver].frequency>>16;
                output_buffer[6]=receiver[current_receiver].frequency>>8;
                output_buffer[7]=receiver[current_receiver].frequency;
                receiver[current_receiver].frequency_changed=0;
            } else {
                output_buffer[3]=control_out[0];
                output_buffer[4]=control_out[1];
                output_buffer[5]=control_out[2];
                output_buffer[6]=control_out[3];
                output_buffer[7]=control_out[4];
            }

            if(ozy_ringbuffer_entries(ozy_output_buffer)>=(OZY_BUFFER_SIZE-8)) {
                bytes=ozy_ringbuffer_get(ozy_output_buffer,&output_buffer[8],OZY_BUFFER_SIZE-8);
                if(bytes!=OZY_BUFFER_SIZE-8) {
                    fprintf(stderr,"OOPS - thought there was enough for usb output but only %d\n",bytes);
                }
            } else {
                memset((char *)&output_buffer[8],0,OZY_BUFFER_SIZE-8);
            }

            bytes=ozy_write(0x02,output_buffer,OZY_BUFFER_SIZE);
            if(bytes!=OZY_BUFFER_SIZE) {
                perror("OzyBulkWrite failed");
            }

if(tx_frame<10) {
    dump_ozy_buffer("sent to Ozy:",tx_frame,output_buffer);
}

            tx_frame++;

        }

        current_receiver++;

        if(current_receiver==receivers) {
            current_receiver=0;
        }


    }
}

void process_ozy_input_buffer(char* buffer) {
    int frame;
    int b=0;
    int b_max;
    int r;
    int left_sample,right_sample,mic_sample;
    float left_sample_float,right_sample_float,mic_sample_float;
    int rc;

    for(frame=0;frame<OZY_BUFFERS;frame++) {
        b=frame*OZY_BUFFER_SIZE;
    if(buffer[b++]==SYNC && buffer[b++]==SYNC && buffer[b++]==SYNC) {

        // extract control bytes
        control_in[0]=buffer[b++];
        control_in[1]=buffer[b++];
        control_in[2]=buffer[b++];
        control_in[3]=buffer[b++];
        control_in[4]=buffer[b++];

        // extract PTT, DOT and DASH
        ptt=(control_in[0]&0x01)==0x01;
        dash=(control_in[0]&0x02)==0x02;
        dot=(control_in[0]&0x04)==0x04;


        if((control_in[0]&0x08)==0) {
            if(control_in[1]&0x01) {
                lt2208ADCOverflow=1;
            }
            if(mercury_software_version!=control_in[2]) {
                mercury_software_version=control_in[2];
                fprintf(stderr,"Mercury Software version: %d (0x%0X)\n",mercury_software_version,mercury_software_version);
            }
            if(penelope_software_version!=control_in[3]) {
                penelope_software_version=control_in[3];
                fprintf(stderr,"Penelope Software version: %d (0x%0X)\n",penelope_software_version,penelope_software_version);
            }
            if(ozy_software_version!=control_in[4]) {
                ozy_software_version=control_in[4];
                fprintf(stderr,"Ozy Software version: %d (0x%0X)\n",ozy_software_version,ozy_software_version);
            }
        } else if(control_in[0]&0x08) {
            forward_power=(control_in[1]<<8)+control_in[2];
        }

        switch(receivers) {
            case 1: b_max=512-0; break;
            case 2: b_max=512-0; break;
            case 3: b_max=512-4; break;
            case 4: b_max=512-10; break;
            case 5: b_max=512-24; break;
            case 6: b_max=512-10; break;
            case 7: b_max=512-20; break;
            case 8: b_max=512-4; break;
        }

        // extract the samples
        while(b<b_max) {
            // extract each of the receivers
            for(r=0;r<receivers;r++) {
                left_sample   = (int)((signed char)buffer[b++]) << 16;
                left_sample  += (int)((unsigned char)buffer[b++]) << 8;
                left_sample  += (int)((unsigned char)buffer[b++]);
                right_sample  = (int)((signed char)buffer[b++]) << 16;
                right_sample += (int)((unsigned char)buffer[b++]) << 8;
                right_sample += (int)((unsigned char)buffer[b++]);
                left_sample_float=(float)left_sample/8388607.0; // 24 bit sample
                right_sample_float=(float)right_sample/8388607.0; // 24 bit sample
                receiver[r].input_buffer[samples]=left_sample_float;
                receiver[r].input_buffer[samples+BUFFER_SIZE]=right_sample_float;
            }
            mic_sample    = (int)((signed char) buffer[b++]) << 8;
            mic_sample   += (int)((unsigned char)buffer[b++]);
            mic_sample_float=(float)mic_sample/32767.0*mic_gain; // 16 bit sample

            // add to buffer
            mic_left_buffer[samples]=mic_sample_float;
            mic_right_buffer[samples]=0.0f;
            samples++;

            sample_count++;
            if(sample_count==sample_rate) {
                ftime(&end_time);
                fprintf(stderr,"%d samples in %ld ms\n",sample_count,((end_time.time*1000)+end_time.millitm)-((start_time.time*1000)+start_time.millitm));
                sample_count=0;
                ftime(&start_time);
            }

            // when we have enough samples send them to the clients
            if(samples==BUFFER_SIZE) {
                // send I/Q data to clients
                for(r=0;r<receivers;r++) {
                    send_IQ_buffer(r);
                }
                samples=0;
            }
        }

    } else {
        fprintf(stderr,"SYNC error\n");
        dump_ozy_buffer("SYNC ERROR",rx_frame,buffer);
        exit(1);
    }
    }
}

void* ozy_ep4_io_thread(void* arg) {
    unsigned char buffer[BANDSCOPE_BUFFER_SIZE*2];
    int bytes;
    int i;

    while(1) {
        bytes=ozy_read(0x84,(void*)(bandscope.buffer),sizeof(buffer));
        if (bytes < 0) {
            fprintf(stderr,"ozy_ep4_io_thread: OzyBulkRead failed %d bytes\n",bytes);
            exit(1);
        } else if (bytes != BANDSCOPE_BUFFER_SIZE*2) {
            fprintf(stderr,"ozy_ep4_io_thread: OzyBulkRead only read %d bytes\n",bytes);
            exit(1);
        } else {
            // process the buffer
            process_bandscope_buffer(buffer);
        }
    }
}


void process_bandscope_buffer(char* buffer) {
    int b=0;
    int i=0;
    int sample;
    float sample_float;

    for(i=0;i<BANDSCOPE_BUFFER_SIZE;i++) {
        sample    = (int)((signed char) buffer[b++]) << 8;
        sample   += (int)((unsigned char)buffer[b++]);
        sample_float=(float)sample/32767.0; // 16 bit sample
        bandscope.buffer[i]=sample_float;
    }

    send_bandscope_buffer();
}

void process_ozy_output_buffer(float *left_output_buffer,float *right_output_buffer) {
    unsigned char ozy_samples[1024*8];
    int j,c;
    short left_rx_sample;
    short right_rx_sample;
    short left_tx_sample;
    short right_tx_sample;

    // process any output
    for(j=0,c=0;j<BUFFER_SIZE;j+=output_sample_increment) {
        left_rx_sample=(short)(left_output_buffer[j]*32767.0);
        right_rx_sample=(short)(right_output_buffer[j]*32767.0);

/*
        if(mox || ptt || dot || dash ) {
            left_tx_sample=(short)(left_tx_buffer[j]*32767.0*rfGain);
            right_tx_sample=(short)(right_tx_buffer[j]*32767.0*rfGain);
        } else {
*/
            left_tx_sample=0;
            right_tx_sample=0;
/*
        }
*/

        ozy_samples[c]=left_rx_sample>>8;
        ozy_samples[c+1]=left_rx_sample;
        ozy_samples[c+2]=right_rx_sample>>8;
        ozy_samples[c+3]=right_rx_sample;
        ozy_samples[c+4]=left_tx_sample>>8;
        ozy_samples[c+5]=left_tx_sample;
        ozy_samples[c+6]=right_tx_sample>>8;
        ozy_samples[c+7]=right_tx_sample;
        c+=8;
/*
        if(c==sizeof(ozy_samples)) {
            if(ozy_ringbuffer_put(ozy_output_buffer,ozy_samples,c)!=c) {
                // ozy output buffer overflow
            }
            c=0;
        }
*/
    }
   ozy_ringbuffer_put(ozy_output_buffer,ozy_samples,c);
}

