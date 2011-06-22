/**
* @file server.c
* @brief HPSDR server application
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
#include <getopt.h>

#include "client.h"
#include "listener.h"
#include "ozy.h"
#include "receiver.h"

static struct option long_options[] = {
    {"receivers",required_argument, 0, 0},
    {"samplerate",required_argument, 0, 1},
};
static char* short_options="rs";
static int option_index;

void process_args(int argc,char* argv[]);

int main(int argc,char* argv[]) {

    process_args(argc,argv);

    init_receivers();

    create_listener_thread();

    create_ozy_thread();

    while(1) {
        ozy_handle_events();
        usleep(200);
    }
}

void process_args(int argc,char* argv[]) {
    int i;

    // set defaults
    ozy_set_receivers(1);
    ozy_set_sample_rate(96000);

    while((i=getopt_long(argc,argv,short_options,long_options,&option_index))!=EOF) {
        switch(option_index) {
            case 0: // receivers
                ozy_set_receivers(atoi(optarg));
                break;
                break;
            case 1: // sample rate
                ozy_set_sample_rate(atoi(optarg));
                break;
        }
    }
}

