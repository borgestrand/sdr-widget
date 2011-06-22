/**
* @file iqthread.c
* @brief thread to receive iq samples
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "iqthread.h"

#define SMALL_PACKETS

static int iq_socket;
static struct sockaddr_in iq_addr;
static int iq_length;
static pthread_t iq_thread_id;
static void (*iq_callback)(void *);

void* iq_thread(void* arg);

void init_iq_thread(int rx) {

    iq_length=sizeof(iq_addr);

    iq_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(iq_socket<0) {
        perror("create iq socket failed");
        exit(1);
    }

    memset(&iq_addr,0,iq_length);
    iq_addr.sin_family=AF_INET;
    iq_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    iq_addr.sin_port=htons(IQPORT+rx);

    if(bind(iq_socket,(struct sockaddr*)&iq_addr,iq_length)<0) {
        perror("bind socket failed for iq socket");
        exit(1);
    }

}

int get_iq_port() {
    return ntohs(iq_addr.sin_port);
}

void start_iq_thread( void(*callback)(void *)) {
    iq_callback=callback;
    if(pthread_create(&iq_thread_id,NULL,iq_thread,NULL)<0) {
        perror("pthread_create failed for iq_thread");
        exit(1);
    }
}

void* iq_thread(void* arg) {
    struct sockaddr_in server_addr;
    int server_length;
    int bytes;
    char buffer[1024*4*2];
    BUFFER small_buffer;
    unsigned long sequence=0L;
    unsigned short offset=0;;
    unsigned short length;


fprintf(stderr,"iq_thread: listening on port %d\n",ntohs(iq_addr.sin_port));
#ifdef SMALL_PACKETS
fprintf(stderr,"SMALL_PACKETS defined\n");
#endif

    while(1) {

#ifdef SMALL_PACKETS
        while(1) {
            bytes=recvfrom(iq_socket,(char*)&small_buffer,sizeof(small_buffer),0,(struct sockaddr*)&server_addr,&server_length);
            if(bytes<0) {
                perror("recvfrom socket failed for spectrum buffer");
                exit(1);
            }

            if(small_buffer.offset==0) {
                offset=0;
                sequence=small_buffer.sequence;
                // start of a frame
                memcpy((char *)&buffer[small_buffer.offset],(char *)&small_buffer.data[0],small_buffer.length);
                offset+=small_buffer.length;
            } else {
                if((sequence==small_buffer.sequence) && (offset==small_buffer.offset)) {
                    memcpy((char *)&buffer[small_buffer.offset],(char *)&small_buffer.data[0],small_buffer.length);
                    offset+=small_buffer.length;
                    if(offset==sizeof(buffer)) {
                        offset=0;
                        break;
                    }
                } else {
                }
            }
        }

#else
        bytes=recvfrom(iq_socket,(char*)buffer,sizeof(buffer),0,(struct sockaddr*)&server_addr,&server_length);
        if(bytes<0) {
            perror("recvfrom failed for iq buffer");
            exit(1);
        }
#endif

        // process the I/Q samples
        iq_callback(buffer);
    }
}
