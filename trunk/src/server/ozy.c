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

#ifdef __linux__
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/timeb.h>
#include <pthread.h>
#else
#include "pthread.h"
#endif

#include "client.h"
#include "ozyio.h"
#include "bandscope.h"
#include "receiver.h"
//#include "transmitter.h"
#include "util.h"
#include "metis.h"

#define THREAD_STACK 32768

#define DEFAULT_OZY_BUFFERS 16
#define OZY_BUFFER_SIZE 512
#define OZY_HEADER_SIZE 8

static int ozy_buffers=DEFAULT_OZY_BUFFERS;

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
static pthread_t playback_thread_id;

static int configure=6;
static int rx_frame=0;
static int tx_frame=0;
static int receivers=1;
static int current_receiver=0;

static int speed=1;
static int sample_rate=96000;
static int output_sample_increment=2;

static int timing=0;
static struct timeb start_time;
static struct timeb end_time;
static int sample_count=0;

static unsigned char control_in[5]={0x00,0x00,0x00,0x00,0x00};
static unsigned char control_out[5]={
  MOX_DISABLED,
  CONFIG_MERCURY | MERCURY_122_88MHZ_SOURCE | MERCURY_10MHZ_SOURCE | MIC_SOURCE_PENELOPE | SPEED_96KHZ,
  MODE_OTHERS,
  ALEX_ATTENUATION_0DB | LT2208_GAIN_OFF | LT2208_DITHER_ON | LT2208_RANDOM_ON,
  DUPLEX
};

static int mox=0;

static int ptt=0;
static int dot=0;
static int dash=0;
static int lt2208ADCOverflow=0;

static char ozy_firmware_version[9];
static int mercury_software_version=0;
static int penelope_software_version=0;
static int ozy_software_version=0;

static int forwardPower=0;
static int alexForwardPower=0;
static int alexReversePower=0;
static int AIN3=0;
static int AIN4=0;
static int AIN6=0;
static int IO1=1; // 1 is inactive
static int IO2=1;
static int IO3=1;


static int samples=0;

static float mic_gain=0.26F;
static float mic_left_buffer[BUFFER_SIZE];
static float mic_right_buffer[BUFFER_SIZE];

static char ozy_firmware[64];
static char ozy_fpga[64];

static unsigned char ozy_output_buffer[OZY_BUFFER_SIZE];
static int ozy_output_buffer_index=OZY_HEADER_SIZE;


static char filename[256];
static int record=0;
static int playback=0;
static int playback_sleep=0;
static FILE* recording;

int metis=0;

void ozy_prime();
void* ozy_ep6_ep2_io_thread(void* arg);
void* ozy_ep4_io_thread(void* arg);
void* playback_thread(void* arg);

void process_ozy_input_buffer(char* buffer);
void write_ozy_output_buffer();
void process_bandscope_buffer(char* buffer);

#ifndef __linux__
#define bool int
bool init_hpsdr();
#endif

void ozy_set_buffers(int buffers) {
    ozy_buffers=buffers;
}

void ozy_set_metis(int state) {
    metis=state;
}

int create_ozy_thread() {
    int i;
    int rc;

#ifndef __linux__
    if (init_hpsdr() == 0) exit(9);
#endif

    for(i=0;i<receivers;i++) {
        receiver[i].frequency=7056000L;
        receiver[i].frequency_changed=1;
    }

    if(!playback) {
        ozy_init();
        ozy_prime();
    }

    if(timing) {
        ftime(&start_time);
    }

    if(playback) {
        // create a thread to read/write to EP6/EP2
        rc=pthread_create(&playback_thread_id,NULL,playback_thread,NULL);
        if(rc != 0) {
            fprintf(stderr,"pthread_create failed on playback_thread: rc=%d\n", rc);
            exit(1);
        }
    } else {
        // create a thread to read/write to EP6/EP2
        rc=pthread_create(&ep6_ep2_io_thread_id,NULL,ozy_ep6_ep2_io_thread,NULL);
        if(rc != 0) {
            fprintf(stderr,"pthread_create failed on ozy_ep6_io_thread: rc=%d\n", rc);
            exit(1);
        }

        // create a thread to read from EP4
        rc=pthread_create(&ep4_io_thread_id,NULL,ozy_ep4_io_thread,NULL);
        if(rc != 0) {
            fprintf(stderr,"pthread_create failed on ozy_ep4_io_thread: rc=%d\n", rc);
            exit(1);
        }
    }

    return 0;
}

void ozy_set_record(char* f) {
    if(playback||record) {
        fclose(recording);
    }
    strcpy(filename,f);
    recording=fopen(filename,"w");
    record=1;
    playback=0;

fprintf(stderr,"recording\n");
}

void ozy_stop_record() {
    if(record) {
        fclose(recording);
    }
    record=0;
fprintf(stderr,"stopped recording\n");
}

void ozy_set_playback(char* f) {
    if(playback||record) {
        fclose(recording);
    }
    strcpy(filename,f);
    recording=fopen(filename,"r");
    playback=1;
    record=0;

fprintf(stderr,"starting playback: %s\n",filename);
}

void ozy_set_receivers(int r) {
    if(r>MAX_RECEIVERS) {
        fprintf(stderr,"MAX Receivers is 8!\n");
        exit(1);
    }
    receivers=r;
    control_out[4] &= 0xc7;
    control_out[4] |= (r-1)<<3;
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
    control_out[1] &= 0xfc;
    control_out[1] |= speed;

    //playback_sleep=(int)((1000000.0/380.0));
    playback_sleep=3090;
    fprintf(stderr,"receivers=%d sample_rate=%d playback_sleep=%d\n",receivers,sample_rate,playback_sleep);

}

int ozy_set_playback_sleep(int sleep) {
    playback_sleep=sleep;
}

int ozy_get_sample_rate() {
    return sample_rate;
}

int ozy_init() {
    int rc;
    int i;

    strcpy(ozy_firmware,"ozyfw-sdr1k.hex");
    strcpy(ozy_fpga,"Ozy_Janus.rbf");

        // On Windows, the following is replaced by init_hpsdr() in OzyInit.c
#ifdef __linux__

    // open ozy
    rc = ozy_open();
    if (rc != 0) {
        fprintf(stderr,"Cannot locate Ozy\n");
        exit(1);
    }

    // load Ozy FW
    ozy_reset_cpu(1);
    ozy_load_firmware(ozy_firmware);
    ozy_reset_cpu(0);
ozy_close();
    sleep(5);
ozy_open();
    ozy_set_led(1,1);
    ozy_load_fpga(ozy_fpga);
    ozy_set_led(1,0);
    ozy_close();

    ozy_open();
    rc=ozy_get_firmware_string(ozy_firmware_version,8);
    fprintf(stderr,"Ozy FX2 version: %s\n",ozy_firmware_version);
#endif


    return rc;
}

void ozy_prime() {
    int i;

    memset((char *)&ozy_output_buffer,0,OZY_BUFFER_SIZE);
    while(configure>0) {
        write_ozy_output_buffer();
    }


    for(i=0;i<receivers;i++) {
        current_receiver=i;
        write_ozy_output_buffer();
    }

    current_receiver=0;

fprintf(stderr,"server configured for %d receivers at %d\n",receivers,sample_rate);
}

void* ozy_ep6_ep2_io_thread(void* arg) {
    unsigned char input_buffer[OZY_BUFFER_SIZE*ozy_buffers];
    int bytes;
    int i;

    while(1) {
        // read an input buffer (blocks until all bytes read)
        bytes=ozy_read(0x86,input_buffer,OZY_BUFFER_SIZE*ozy_buffers);
        if (bytes < 0) {
            fprintf(stderr,"ozy_ep6_ep2_io_thread: OzyBulkRead read failed %d\n",bytes);
        } else if (bytes != OZY_BUFFER_SIZE*ozy_buffers) {
            fprintf(stderr,"ozy_ep6_ep2_io_thread: OzyBulkRead only read %d bytes\n",bytes);
        } else {
            // process input buffers
            for(i=0;i<ozy_buffers;i++) {
                process_ozy_input_buffer(&input_buffer[i*OZY_BUFFER_SIZE]);
            }
            if(record) {
                bytes=fwrite(input_buffer,sizeof(char),OZY_BUFFER_SIZE*ozy_buffers,recording);
            }
        }

        current_receiver++;

        if(current_receiver==receivers) {
            current_receiver=0;
        }
    }
}

void* playback_thread(void* arg) {
    unsigned char input_buffer[OZY_BUFFER_SIZE*ozy_buffers];
    int bytes;
    int i;

    while(1) {
        // read an input buffer (blocks until all bytes read)
        bytes=fread(input_buffer,sizeof(char), OZY_BUFFER_SIZE*ozy_buffers,recording);
        if (bytes <= 0) {
            fclose(recording);
fprintf(stderr,"restarting playback: %s\n",filename);
            recording=fopen(filename,"r");
            bytes=fread(input_buffer,sizeof(char), OZY_BUFFER_SIZE*ozy_buffers,recording);
        }
        // process input buffers
        for(i=0;i<ozy_buffers;i++) {
            process_ozy_input_buffer(&input_buffer[i*OZY_BUFFER_SIZE]);
        }

        current_receiver++;

        if(current_receiver==receivers) {
            current_receiver=0;
        }
        usleep(playback_sleep);
    }
}

void write_ozy_output_buffer() {
    int bytes;

    ozy_output_buffer[0]=SYNC;
    ozy_output_buffer[1]=SYNC;
    ozy_output_buffer[2]=SYNC;

    if(configure>0) {
        configure--;
        ozy_output_buffer[3]=control_out[0];
        ozy_output_buffer[4]=control_out[1];
        ozy_output_buffer[5]=control_out[2];
        ozy_output_buffer[6]=control_out[3];
        ozy_output_buffer[7]=control_out[4];
    } else if(receiver[current_receiver].frequency_changed) {
        ozy_output_buffer[3]=control_out[0]|((current_receiver+2)<<1);
        ozy_output_buffer[4]=receiver[current_receiver].frequency>>24;
        ozy_output_buffer[5]=receiver[current_receiver].frequency>>16;
        ozy_output_buffer[6]=receiver[current_receiver].frequency>>8;
        ozy_output_buffer[7]=receiver[current_receiver].frequency;
        receiver[current_receiver].frequency_changed=0;
    } else {
        ozy_output_buffer[3]=control_out[0];
        ozy_output_buffer[4]=control_out[1];
        ozy_output_buffer[5]=control_out[2];
        ozy_output_buffer[6]=control_out[3];
        ozy_output_buffer[7]=control_out[4];
    }

    if(metis) {
        bytes=metis_write(0x02,ozy_output_buffer,OZY_BUFFER_SIZE);
        if(bytes!=OZY_BUFFER_SIZE) {
            perror("OzyBulkWrite failed");
        }
    } else {
        bytes=ozy_write(0x02,ozy_output_buffer,OZY_BUFFER_SIZE);
        if(bytes!=OZY_BUFFER_SIZE) {
            perror("OzyBulkWrite failed");
        }
    }

if(tx_frame<10) {
    dump_ozy_buffer("sent to Ozy:",tx_frame,ozy_output_buffer);
}
    tx_frame++;

}

void process_ozy_input_buffer(char* buffer) {
    int b=0;
    int b_max;
    int r;
    int left_sample,right_sample,mic_sample;
    float left_sample_float,right_sample_float,mic_sample_float;
    int rc;
    int i;
    int bytes;

if(rx_frame<10) {
    dump_ozy_buffer("received from Ozy:",rx_frame,buffer);
}

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

        switch((control_in[0]>>3)&0x1F) {

        case 0:
            lt2208ADCOverflow=control_in[1]&0x01;
            IO1=(control_in[1]&0x02)?0:1;
            IO2=(control_in[1]&0x04)?0:1;
            IO3=(control_in[1]&0x08)?0:1;
            if(mercury_software_version!=control_in[2]) {
                mercury_software_version=control_in[2];
                fprintf(stderr,"  Mercury Software version: %d (0x%0X)\n",mercury_software_version,mercury_software_version);
            }
            if(penelope_software_version!=control_in[3]) {
                penelope_software_version=control_in[3];
                fprintf(stderr,"  Penelope Software version: %d (0x%0X)\n",penelope_software_version,penelope_software_version);
            }
            if(ozy_software_version!=control_in[4]) {
                ozy_software_version=control_in[4];
                fprintf(stderr,"  Ozy Software version: %d (0x%0X)\n",ozy_software_version,ozy_software_version);
            }
            break;
        case 1:
            forwardPower=(control_in[1]<<8)+control_in[2]; // from Penelope or Hermes

            alexForwardPower=(control_in[3]<<8)+control_in[4]; // from Alex or Apollo
            break;
        case 2:
            alexForwardPower=(control_in[1]<<8)+control_in[2]; // from Alex or Apollo
            AIN3=(control_in[3]<<8)+control_in[4]; // from Pennelope or Hermes
            break;
        case 3:
            AIN4=(control_in[1]<<8)+control_in[2]; // from Pennelope or Hermes
            AIN6=(control_in[3]<<8)+control_in[4]; // from Pennelope or Hermes
            break;
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

            if(timing) {
                sample_count++;
                if(sample_count==sample_rate) {
                    ftime(&end_time);
                    fprintf(stderr,"%d samples in %ld ms\n",sample_count,((end_time.time*1000)+end_time.millitm)-((start_time.time*1000)+start_time.millitm));
                    sample_count=0;
                    ftime(&start_time);
                }
            }


            // when we have enough samples send them to the clients
            if(samples==BUFFER_SIZE) {
                //if(ptt||mox) {
                //    process_microphone_samples(mic_left_buffer);
                //}
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


    rx_frame++;
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
//fprintf(stderr,"%d: %d %f\n",i,sample,sample_float);
    }

    send_bandscope_buffer();
}

void process_ozy_output_buffer(float *left_output_buffer,float *right_output_buffer,int mox) {
    unsigned char ozy_samples[1024*8];
    int j,c;
    short left_rx_sample;
    short right_rx_sample;
    short left_tx_sample;
    short right_tx_sample;

    if(!playback) {
        // process the output
        for(j=0,c=0;j<BUFFER_SIZE;j+=output_sample_increment) {

            if(mox) {
                left_tx_sample=(short)(left_output_buffer[j]*32767.0);
                right_tx_sample=(short)(right_output_buffer[j]*32767.0);
                left_rx_sample=0;
                right_rx_sample=0;
            } else {
                left_rx_sample=(short)(left_output_buffer[j]*32767.0);
                right_rx_sample=(short)(right_output_buffer[j]*32767.0);
                left_tx_sample=0;
                right_tx_sample=0;
            }

            ozy_output_buffer[ozy_output_buffer_index++]=left_rx_sample>>8;
            ozy_output_buffer[ozy_output_buffer_index++]=left_rx_sample;
            ozy_output_buffer[ozy_output_buffer_index++]=right_rx_sample>>8;
            ozy_output_buffer[ozy_output_buffer_index++]=right_rx_sample;
            ozy_output_buffer[ozy_output_buffer_index++]=left_tx_sample>>8;
            ozy_output_buffer[ozy_output_buffer_index++]=left_tx_sample;
            ozy_output_buffer[ozy_output_buffer_index++]=right_tx_sample>>8;
            ozy_output_buffer[ozy_output_buffer_index++]=right_tx_sample;

            if(ozy_output_buffer_index==OZY_BUFFER_SIZE) {
                write_ozy_output_buffer();
                ozy_output_buffer_index=OZY_HEADER_SIZE;
            }
        }
    }

}

void ozy_set_preamp(int p) {
    control_out[3]=control_out[3]&0xFB;
    control_out[3]=control_out[3]|(p<<2);
}

void ozy_set_dither(int dither) {
    control_out[3]=control_out[3]&0xF7;
    control_out[3]=control_out[3]|(dither<<3);
}

void ozy_set_random(int random) {
    control_out[3]=control_out[3]&0xEF;
    control_out[3]=control_out[3]|(random<<4);
}

void ozy_set_10mhzsource(int source) {
    control_out[1]=control_out[1]&0xF3;
    control_out[1]=control_out[1]|(source<<2);
}

void ozy_set_122_88mhzsource(int source) {
    control_out[1]=control_out[1]&0xEF;
    control_out[1]=control_out[1]|(source<<4);
}

void ozy_set_micsource(int source) {
    control_out[1]=control_out[1]&0x7F;
    control_out[1]=control_out[1]|(source<<7);
}

void ozy_set_class(int c) {
    control_out[2]=control_out[2]&0xFE;
    control_out[2]=control_out[2]|c;
}

void ozy_set_timing(int t) {
    timing=t;
}

void ozy_set_open_collector_outputs(int oc) {
    control_out[2]=control_out[2]&0x01;
    control_out[2]=control_out[2]|(oc<<1);
}
