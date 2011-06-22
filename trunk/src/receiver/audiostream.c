/**
* @file audiostream.c
* @brief audio out put stream (for iPhone)
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-03-10
*/

/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
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

#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>


#include "audiostream.h"

static int sample_count=0;

unsigned char audio_stream_buffer[512];
int audio_stream_buffer_insert=0;
int audio_stream_buffer_remove=0;

unsigned char encodetable[65536];

unsigned char alaw(short sample);

static pthread_t audio_stream_thread_id;

int audio_stream_port=8002;

int audio_stream_serverSocket;
int audio_stream_clientSocket=-1;
struct sockaddr_in audio_stream_server;
struct sockaddr_in audio_stream_client;
int audio_stream_addrlen;

void* audio_stream_thread(void* arg);
void init_alaw_tables();

void audio_stream_init() {

    int rc;

    init_alaw_tables();

    audio_stream_clientSocket=-1;
    rc=pthread_create(&audio_stream_thread_id,NULL,audio_stream_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on audio_stream_thread: rc=%d\n", rc);
    }

}

void* audio_stream_thread(void* arg) {

    int bytesRead;
    char message[64];
    int on=1;

fprintf(stderr,"audio_stream_thread\n");

    audio_stream_serverSocket=socket(AF_INET,SOCK_STREAM,0);
    if(audio_stream_serverSocket==-1) {
        perror("audio_stream socket");
        return;
    }

    setsockopt(audio_stream_serverSocket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&audio_stream_server,0,sizeof(audio_stream_server));
    audio_stream_server.sin_family=AF_INET;
    audio_stream_server.sin_addr.s_addr=INADDR_ANY;
    audio_stream_server.sin_port=htons(audio_stream_port);

    if(bind(audio_stream_serverSocket,(struct sockaddr *)&audio_stream_server,sizeof(audio_stream_server))<0) {
        perror("audio_stream bind");
        return;
    }

    while(1) {
//fprintf(stderr,"audio_stream_thread: listen\n");
        if (listen(audio_stream_serverSocket, 5) == -1) {
            perror("audio_stream listen");
            break;
        }

//fprintf(stderr,"audio_stream_thread: accept\n");
        audio_stream_addrlen = sizeof(audio_stream_client);
        if ((audio_stream_clientSocket = accept(audio_stream_serverSocket,(struct sockaddr *)&audio_stream_client,&audio_stream_addrlen)) == -1) {
                perror("audio_stream accept");
        } else {

            time_t tt;
            struct tm *tod;

            time(&tt);
            tod=localtime(&tt);
            fprintf(stderr,"%02d/%02d/%02d %02d:%02d:%02d audiostream connection from %s:%d\n",tod->tm_mday,tod->tm_mon+1,tod->tm_year+1900,tod->tm_hour,tod->tm_min,tod->tm_sec,inet_ntoa(audio_stream_client.sin_addr),ntohs(audio_stream_client.sin_port));

            while(1) {
                bytesRead=recv(audio_stream_clientSocket, message, sizeof(message), 0);
                if(bytesRead<=0) {
                    //perror("audio_stream recv");
                    break;
                }
                message[bytesRead]=0;

            }

            close(audio_stream_clientSocket);

            time(&tt);
            tod=localtime(&tt);
            fprintf(stderr,"%02d/%02d/%02d %02d:%02d:%02d audiostream disconnected from %s:%d\n",tod->tm_mday,tod->tm_mon+1,tod->tm_year+1900,tod->tm_hour,tod->tm_min,tod->tm_sec,inet_ntoa(audio_stream_client.sin_addr),ntohs(audio_stream_client.sin_port));
        }
        audio_stream_clientSocket=-1;

    }

}

void send_audio_buffer(unsigned char* buffer,int length) {
    int rc;
    if(audio_stream_clientSocket!=-1) {
//fprintf(stderr,"audio_stream_send_samples\n");
        rc=send(audio_stream_clientSocket,buffer,length,0);
        if(rc<0) {
            perror("audiostream send");
        }
    }

}

/* --------------------------------------------------------------------------*/
/**
* @brief put samples to the audio stream
*
* @return
*/

void audio_stream_put_samples(short left_sample,short right_sample) {

    // samples are delivered at 48K
    // output to stream at 8K (1 in 6)
    if(sample_count==0) {
        // use this sample and convert to a-law (mono)
        audio_stream_buffer[audio_stream_buffer_insert]=alaw((left_sample+right_sample)/2);
        //audio_stream_buffer[audio_stream_buffer_insert]=(left_sample+right_sample)/2;
//        audio_stream_buffer[audio_stream_buffer_insert]=left_sample;
//        audio_stream_buffer_insert++;
        //audio_stream_buffer[audio_stream_buffer_insert]=((left_sample+right_sample)/2)>>8;
        //audio_stream_buffer[audio_stream_buffer_insert]=left_sample>>8;
        audio_stream_buffer_insert++;
        if(audio_stream_buffer_insert==256) {
            send_audio_buffer(&audio_stream_buffer[0],256);
        } else if(audio_stream_buffer_insert==512) {
            send_audio_buffer(&audio_stream_buffer[256],256);
            audio_stream_buffer_insert=0;
        }
    }
    sample_count++;
    if(sample_count==6) {
        sample_count=0;
    }
}

unsigned char alaw(short sample) {
    return encodetable[sample&0xFFFF];
}

void init_alaw_tables() {
    int i;

/*
    for (i = 0; i < 256; i++) {
        int input = i ^ 85;
        int mantissa = (input & 15) << 4;
        int segment = (input & 112) >> 4;
        int value = mantissa + 8;
        if (segment >= 1) value += 256;
        if (segment > 1) value <<= (segment - 1);
        if ((input & 128) == 0) value = -value;
        decodetable[i]=(short)value;
    }
*/
    for(i=0;i<65536;i++) {
        short sample=(short)i;

        int sign=(sample&0x8000) >> 8;
        if(sign != 0){
            sample=(short)-sample;
            sign=0x80;
        }

        if(sample > 32635) sample = 32635;

        int exp=7;
        int expMask;
        for(expMask=0x4000;(sample&expMask)==0 && exp>0; exp--, expMask >>= 1) {
        }
        int mantis = (sample >> ((exp == 0) ? 4 : (exp + 3))) & 0x0f;
        unsigned char alaw = (unsigned char)(sign | exp << 4 | mantis);
        encodetable[i]=(unsigned char)(alaw^0xD5);
    }

}

