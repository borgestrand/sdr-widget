/**
* @file softrock.h
* @brief Softrock implementation
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
#if !defined __SOFTROCK_H__
#define __SOFTROCK_H__

//#define PULSEAUDIO
//#define PORTAUDIO
//#define DIRECTAUDIO
#define JACKAUDIO

#ifdef JACKAUDIO
#include <jack/jack.h>
#endif
int create_softrock_thread(void);
void softrock_set_device(char* d);
char* softrock_get_device(void);
void softrock_set_receivers(int r);
int softrock_get_receivers(void);
void softrock_set_sample_rate(int r);
int softrock_get_sample_rate();

void softrock_set_input(char* d);
char* softrock_get_input(void);
void softrock_set_output(char* d);
char* softrock_get_output(void);

void softrock_set_iq(int s);
int softrock_get_iq(void);

void softrock_set_record(char* filename);
void softrock_set_playback(char* filename);
void softrock_record_buffer(char* buffer,int length);
void softrock_playback_buffer(char* buffer,int length);

int softrock_get_record(void);
int softrock_get_playback(void);

void process_softrock_output_buffer(float* left_output_buffer,float* right_output_buffer);

#ifdef JACKAUDIO
int init_jack_audio(void);
void jack_cleanup(void);
int process(jack_nframes_t number_of_frames, void *arg);
void jack_shutdown (void *arg);
#endif

#endif
