/** 
* @file ozy_ringbuffer.c
* @brief Ozy Ring Buffer functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-01-05
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
#include <string.h>
#include <pthread.h>

//#include "ozy_buffers.h"
#include "ozy_ringbuffer.h"

/*
 * 
 */

struct ozy_ringbuffer* ozy_output_buffer = NULL;

pthread_mutex_t ozy_output_buffer_mutex; 

int ozy_put_bytes=0;
int ozy_get_bytes=0;

/* --------------------------------------------------------------------------*/
/** 
* @brief New Ozy ringbuffer
*/
struct ozy_ringbuffer* new_ozy_ringbuffer(int n) {
    struct ozy_ringbuffer* buffer;

    buffer=calloc(1,sizeof(struct ozy_ringbuffer));
    if(buffer!=NULL) {
        buffer->size=n;
        buffer->entries=0;
        buffer->buffer=calloc(1,sizeof(char)*n);
        buffer->insert_index=0;
        buffer->remove_index=0;
    }
    return buffer;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Ozy ringbuffer space
*/
int ozy_ringbuffer_space(struct ozy_ringbuffer* buffer) {
    return buffer->size-buffer->entries;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Ozy ringbuffer entries
*/
int ozy_ringbuffer_entries(struct ozy_ringbuffer* buffer) {
    return buffer->entries;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Ozy ringbuffer put
*/
int ozy_ringbuffer_put(struct ozy_ringbuffer* buffer,unsigned char* f,int n) {
    int bytes;

    pthread_mutex_lock(&ozy_output_buffer_mutex);
    bytes=n;

    if(ozy_ringbuffer_space(buffer)<=n) {
fprintf(stderr,"ozy_ringbuffer_put: space=%d wanted=%d\n",ozy_ringbuffer_space(buffer),n);
        bytes=ozy_ringbuffer_space(buffer)-1;
    }
    ozy_put_bytes+=bytes;
    
    if(bytes>0) {

        if((buffer->insert_index+bytes)<=buffer->size) {
            // all together
            memcpy(&buffer->buffer[buffer->insert_index],f,bytes);
        } else {
            memcpy(&buffer->buffer[buffer->insert_index],f,buffer->size-buffer->insert_index);
            memcpy(buffer->buffer,&f[buffer->size-buffer->insert_index],bytes-(buffer->size-buffer->insert_index));
        }

        buffer->entries+=bytes;
        buffer->insert_index+=bytes;
        if(buffer->insert_index>=buffer->size) {
            buffer->insert_index-=buffer->size;
        }
    }
    pthread_mutex_unlock(&ozy_output_buffer_mutex);

    return n;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Ozy ringbuffer get
*/
int ozy_ringbuffer_get(struct ozy_ringbuffer* buffer,unsigned char* f,int n) {
    int entries;

    pthread_mutex_lock(&ozy_output_buffer_mutex);
    entries=n;
    if(buffer->entries<n) entries=buffer->entries;
    
    ozy_get_bytes+=entries;

    if((buffer->remove_index+entries)<=buffer->size) {
        // all together
        memcpy(f,&buffer->buffer[buffer->remove_index],entries);
    } else {
        memcpy(f,&buffer->buffer[buffer->remove_index],buffer->size-buffer->remove_index);
        memcpy(&f[buffer->size-buffer->remove_index],buffer->buffer,entries-(buffer->size-buffer->remove_index));
    }
    
    buffer->entries-=entries;
    buffer->remove_index+=entries;
    if(buffer->remove_index>=buffer->size) {
        buffer->remove_index-=buffer->size;
    }
    pthread_mutex_unlock(&ozy_output_buffer_mutex);

    return entries;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Create an Ozy ringbuffer
* 
* @param n
* 
* @return 
*/
void create_ozy_ringbuffer(int n) {
    pthread_mutex_init(&ozy_output_buffer_mutex, NULL);
    ozy_output_buffer=new_ozy_ringbuffer(n);
}
