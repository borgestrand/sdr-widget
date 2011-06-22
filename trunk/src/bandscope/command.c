/**
* @file command.c
* @brief command interface to server
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
#include <string.h>

#include "command.h"

static int server_socket;
static int server_length;
static struct sockaddr_in server; 

static char response[80];

void init_command(char* address) {

    server_socket=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(server_socket<0) {
        perror("create server_socket fialed");
        exit(1);
    }

    server_length=sizeof(server);
    memset(&server,0,server_length);
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=inet_addr(address);
    server.sin_port=htons(SERVER_PORT);

fprintf(stderr,"connect: %s:%d\n",address,SERVER_PORT);
    if(connect(server_socket,(struct sockaddr*)&server,server_length)<0) {
        perror("connect failed for server socket");
        exit(1);
    }
}

char* send_command(char* command) {
    int bytes_read;

    // send the command
    if(send(server_socket,command,strlen(command),0)<0) {
        perror("command send failed");
        exit(1);
    }

    // get the response
    bytes_read=recv(server_socket,response,sizeof(response)-1,0);
    if(bytes_read<=0) {
        perror("reading command response");
        exit(1);
    }

    response[bytes_read]='\0';

    if(strncmp(response,"OK",2)!=0) {
        fprintf(stderr,"command error: %s: %s\n",command,response);
    }

    return response;

}

void close_command() {
    close(server_socket);
}

