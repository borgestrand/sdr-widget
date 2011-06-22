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

#define PULSEAUDIO
//#define PORTAUDIO
//#define DIRECTAUDIO

int create_softrock_thread();
void softrock_set_device(char* d);
char* softrock_get_device();
void softrock_set_receivers(int r);
int softrock_get_receivers();
void softrock_set_sample_rate(int r);
int softrock_get_sample_rate();

void softrock_set_input(char* d);
char* softrock_get_input();
void softrock_set_output(char* d);
char* softrock_get_output();

void softrock_set_iq(int s);
int softrock_get_iq();

void softrock_set_record(char* filename);
void softrock_set_playback(char* filename);
void softrock_record_buffer(char* buffer,int length);
void softrock_playback_buffer(char* buffer,int length);

int softrock_get_record();
int softrock_get_playback();
