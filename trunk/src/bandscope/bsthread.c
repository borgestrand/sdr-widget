/**
* @file bsthread.c
* @brief thread to receive bs samples
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

#include "bsthread.h"

static int bs_socket;
static struct sockaddr_in bs_addr;
static int bs_length;
static pthread_t bs_thread_id;
static void (*bs_callback)(void *);

void* bs_thread(void* arg);

void init_bs_thread() {

    bs_length=sizeof(bs_addr);

    bs_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(bs_socket<0) {
        perror("create bs socket failed");
        exit(1);
    }

    memset(&bs_addr,0,bs_length);
    bs_addr.sin_family=AF_INET;
    bs_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    bs_addr.sin_port=htons(BSPORT);

    if(bind(bs_socket,(struct sockaddr*)&bs_addr,bs_length)<0) {
        perror("bind socket failed for bs socket");
        exit(1);
    }

}

int get_bs_port() {
    return ntohs(bs_addr.sin_port);
}

void start_bs_thread( void(*callback)(void *)) {
    bs_callback=callback;
    if(pthread_create(&bs_thread_id,NULL,bs_thread,NULL)<0) {
        perror("pthread_create failed for bs_thread");
        exit(1);
    }
}

void* bs_thread(void* arg) {
    struct sockaddr_in server_addr;
    int server_length;
    int bytes;
    char buffer[1024*4*2];

fprintf(stderr,"bs_thread: listening on port %d\n",ntohs(bs_addr.sin_port));

    while(1) {
        bytes=recvfrom(bs_socket,(char*)buffer,sizeof(buffer),0,(struct sockaddr*)&server_addr,&server_length);
        if(bytes<0) {
            perror("recvfrom failed for bs buffer");
            exit(1);
        }

        // process the I/Q samples
        bs_callback(buffer);
    }
}
