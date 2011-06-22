/**
* @file ozyio.c
* @brief USB I/O with Ozy
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


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ozyio.h"

#define OZY_PID (0x0007)
#define OZY_VID (0xfffe)

#define VRQ_SDR1K_CTL 0x0d
#define SDR1KCTRL_READ_VERSION  0x7
#define VRT_VENDOR_IN 0xC0

#define OZY_IO_TIMEOUT 500

static int init=0;

static int ozy_fd;

int ozy_open() {
    ozy_fd=open("/dev/hpsdr0",O_RDWR);
    if(ozy_fd<0) {
        perror("open /dev/hpsdr0");
        exit(1);
    }
    return 0;
}

int ozy_close() {
    close(ozy_fd);
    return 0;
}

int ozy_get_firmware_string(char* buffer,int buffer_size) {
    strcpy(buffer,"Unknown");
    return 0;
}

int ozy_write(int ep,unsigned char* buffer,int buffer_size) {
    int rc;
    int bytes;

    bytes=write(ozy_fd,buffer,buffer_size);
    if(bytes!=buffer_size) {
        fprintf(stderr,"ozy_write: %d bytes!\n",bytes);
    }
    return bytes;
}

int ozy_read(int ep,unsigned char* buffer,int buffer_size) {
    int rc;
    int bytes;

    bytes=read(ozy_fd,buffer,buffer_size);
    if(bytes!=buffer_size) {
        fprintf(stderr,"ozy_read: %d bytes!\n",bytes);
    }
    return bytes;
}
