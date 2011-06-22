/**
* @file receiver.c
* @brief manage client attachment to receivers
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#include "client.h"
#include "receiver.h"
#include "messages.h"
#include "ozy.h"

RECEIVER receiver[MAX_RECEIVERS];
static int iq_socket;
static struct sockaddr_in iq_addr;
static int iq_length;

static char response[80];

void init_receivers() {
    int i;
    for(i=0;i<MAX_RECEIVERS;i++) {
        receiver[i].client=(CLIENT*)NULL;
    }

    iq_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(iq_socket<0) {
        perror("create socket failed for iq_socket\n");
        exit(1);
    }

    iq_length=sizeof(iq_addr);
    memset(&iq_addr,0,iq_length);
    iq_addr.sin_family=AF_INET;
    iq_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    iq_addr.sin_port=htons(11002);

    if(bind(iq_socket,(struct sockaddr*)&iq_addr,iq_length)<0) {
        perror("bind socket failed for iq socket");
        exit(1);
    }

}

char* attach_receiver(int rx,CLIENT* client) {

    if(client->state==RECEIVER_ATTACHED) {
        return CLIENT_ATTACHED;
    }

    if(rx>=ozy_get_receivers()) {
        return RECEIVER_INVALID;
    }

    if(receiver[rx].client!=(CLIENT *)NULL) {
        return RECEIVER_IN_USE;
    }
    
    client->state=RECEIVER_ATTACHED;
    receiver[rx].client=client;
    client->receiver=rx;;

    sprintf(response,"%s %d",OK,ozy_get_sample_rate());

    return response;
}

char* detach_receiver(int rx,CLIENT* client) {
    if(client->state==RECEIVER_DETACHED) {
        return CLIENT_DETACHED;
    }

    if(rx>=ozy_get_receivers()) {
        return RECEIVER_INVALID;
    }

    if(receiver[rx].client!=client) {
        return RECEIVER_NOT_OWNER;
    }

    client->state=RECEIVER_DETACHED;
    receiver[rx].client=(CLIENT*)NULL;

    return OK;
}

char* set_frequency(int rx,CLIENT* client,long frequency) {
    if(client->state==RECEIVER_DETACHED) {
        return CLIENT_DETACHED;
    }

    if(rx>=ozy_get_receivers()) {
        return RECEIVER_INVALID;
    }

    if(receiver[rx].client!=client) {
        return RECEIVER_NOT_OWNER;
    }

    receiver[rx].frequency=frequency;
    receiver[rx].frequency_changed=1;

    return OK;
}

void send_IQ_buffer(int rx) {
    struct sockaddr_in client;
    int client_length;
    int rc;

    if(rx>=ozy_get_receivers()) {
        fprintf(stderr,"send_spectrum_buffer: invalid rx: %d\n",rx);
        return;
    }

    if(receiver[rx].client!=(CLIENT*)NULL) {
        if(receiver[rx].client->iq_port!=-1) {
            // send the IQ buffer

            client_length=sizeof(client);
            memset((char*)&client,0,client_length);
            client.sin_family=AF_INET;
            client.sin_addr.s_addr=receiver[rx].client->address.sin_addr.s_addr;
            client.sin_port=htons(receiver[rx].client->iq_port);

            rc=sendto(iq_socket,receiver[rx].input_buffer,sizeof(receiver[rx].input_buffer),0,(struct sockaddr*)&client,client_length);

            if(rc<=0) {
                perror("sendto failed for iq data");
                exit(1);
            }
 
        }
    }
}

