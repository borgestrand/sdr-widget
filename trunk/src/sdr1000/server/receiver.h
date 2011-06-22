/**
* @file receiver.h
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

#define MAX_RECEIVERS 8

#define BUFFER_SIZE 1024

typedef struct _receiver {
    int id;
    int audio_socket;
    pthread_t audio_thread_id;
    CLIENT* client;
    int frequency_changed;
    long frequency;
    float input_buffer[BUFFER_SIZE*2];
    float output_buffer[BUFFER_SIZE*2];
} RECEIVER;

typedef struct _buffer {
    unsigned long long sequence;
    unsigned short offset;
    unsigned short length;
    unsigned char data[500];
} BUFFER;

RECEIVER receiver[MAX_RECEIVERS];

#ifdef __cplusplus
 extern "C" {
#endif
extern void init_receivers();
extern char* attach_receiver(int rx,CLIENT* client);
extern char* detach_receiver(int rx,CLIENT* client);
extern char* set_frequency(CLIENT* client,long f);
extern void send_IQ_buffer(int rx);
#ifdef __cplusplus
 }
#endif
