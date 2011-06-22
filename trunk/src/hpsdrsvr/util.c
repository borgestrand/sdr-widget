/** 
* @file util.c
* @brief Utility functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-03-05
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

#include <stdio.h>
#include <stdlib.h>

#define OZY_BUFFER_SIZE 512

/*
 * 
 */

/* --------------------------------------------------------------------------*/
/** 
* @brief Dump Ozy buffer
* 
* @param prefix
* @param buffer
*/
void dump_ozy_buffer(char* prefix,int frame,unsigned char* buffer) {
    int i;
    fprintf(stderr, "%s(%d)\n",prefix,frame);
    for(i=0;i<OZY_BUFFER_SIZE;i+=16) {
        fprintf(stderr, "  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
                i,
                buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
                buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
                );
    }
    fprintf(stderr,"\n");
}

void dump_ozy_header(char* prefix,int frame,unsigned char* buffer) {
    int i;
    fprintf(stderr, "%s(%d)\n",prefix,frame);
    i=0;
    fprintf(stderr, "  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
            i,
            buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
            buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
            );
    fprintf(stderr,"\n");
}
