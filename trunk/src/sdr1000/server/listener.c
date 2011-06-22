/**
* @file listener.c
* @brief Listen for client TCP connections
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>

#include "client.h"
#include "listener.h"
#include "receiver.h"

void* listener_thread(void* arg);
void* client_thread(void* arg);
char* parse_command(CLIENT* client,char* command);

void create_listener_thread() {

    pthread_t thread_id;
    int rc;

    // create the thread to listen for TCP connections
    rc=pthread_create(&thread_id,NULL,listener_thread,NULL);
    if(rc<0) {
        perror("pthread_create listener_thread failed");
        exit(1);
    }
    
}

void* listener_thread(void* arg) {
    int s;
    int address_length;
    struct sockaddr_in address;
    CLIENT* client;
    int rc;

    // create TCP socket to listen on
    s=socket(AF_INET,SOCK_STREAM,0);
    if(s<0) {
        perror("Listen socket failed");
        exit(1);
    }

    // bind to listening port
    memset(&address,0,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(LISTEN_PORT);
    if(bind(s,(struct sockaddr*)&address,sizeof(address))<0) {
        perror("Command bind failed");
        exit(1);
    }

fprintf(stderr,"Listening for TCP connections on port %d\n",LISTEN_PORT);

    while(1) {

        if(listen(s,5)<0) {
            perror("Command listen failed");
            exit(1);
        }

        client=(CLIENT*)malloc(sizeof(CLIENT));
        client->address_length=sizeof(client->address);
        client->iq_port=-1;

        if((client->socket=accept(s,(struct sockaddr*)&client->address,(socklen_t*)&client->address_length))<0) {
            perror("Command accept failed");
            exit(1);
        }
        
fprintf(stderr,"client socket %d\n",client->socket);
fprintf(stderr,"client connected: %s:%d\n",inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));

        rc=pthread_create(&client->thread_id,NULL,client_thread,(void *)client);
        if(rc<0) {
            perror("pthread_create command_thread failed");
            exit(1);
        }

    }
}
