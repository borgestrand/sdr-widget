/**
* @file bandscope.c
* @brief manage client attachment to bandscope
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

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <string.h>

#include "buffer.h"
#include "client.h"
#include "bandscope.h"
#include "messages.h"
#include "ozy.h"

BANDSCOPE bandscope;
static int bs_socket;
static struct sockaddr_in bs_addr;
static int bs_length;

static unsigned long sequence=0L;

static char response[80];

void init_bandscope() {
    bandscope.client=(CLIENT*)NULL;

    bs_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(bs_socket<0) {
        perror("create socket failed for bs_socket\n");
        exit(1);
    }

    bs_length=sizeof(bs_addr);
    memset(&bs_addr,0,bs_length);
    bs_addr.sin_family=AF_INET;
    bs_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    bs_addr.sin_port=htons(11004);

    if(bind(bs_socket,(struct sockaddr*)&bs_addr,bs_length)<0) {
        perror("bind socket failed for bs socket");
        exit(1);
    }

}

char* attach_bandscope(CLIENT* client) {

    if(bandscope.client!=(CLIENT *)NULL) {
        return BANDSCOPE_IN_USE;
    }
    
    bandscope.client=client;

    return OK;
}

char* detach_bandscope(CLIENT* client) {

    if(bandscope.client!=client) {
        return BANDSCOPE_NOT_OWNER;
    }

    bandscope.client=(CLIENT*)NULL;

    return OK;
}

void send_bandscope_buffer() {
    struct sockaddr_in client;
    int client_length;
    unsigned short offset;
    unsigned short length;
    BUFFER buffer;
    int rc;

    if(bandscope.client!=(CLIENT*)NULL) {
        if(bandscope.client->bs_port!=-1) {
            // send the Bandscope buffer

            client_length=sizeof(client);
            memset((char*)&client,0,client_length);
            client.sin_family=AF_INET;
            client.sin_addr.s_addr=bandscope.client->address.sin_addr.s_addr;
            client.sin_port=htons(bandscope.client->bs_port);

            // keep UDP packets to 512 bytes or less
            //     8 bytes sequency number
            //     2 byte offset
            //     2 byte length
            offset=0;
            while(offset<sizeof(bandscope.buffer)) {
                buffer.sequence=sequence;
                buffer.offset=offset;
                buffer.length=sizeof(bandscope.buffer)-offset;
                if(buffer.length>500) buffer.length=500;
                memcpy((char*)&buffer.data[0],(char*)&bandscope.buffer[offset/4],buffer.length);
                rc=sendto(bs_socket,(char*)&buffer,sizeof(buffer),0,(struct sockaddr*)&client,client_length);
                if(rc<=0) {
                    perror("sendto failed for bandscope data");
                    exit(1);
                }
                offset+=buffer.length;
            }
            sequence++;

        }
    }
}
