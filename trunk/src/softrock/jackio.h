/**
* @file jackio.h
* @brief Softrock implementation
* @author Rob Frohne, KL7NA "at" arrl "dot" net
* @version 0.1
* @date 20011-09-05
*/

/* Copyright (C)
* 2011 Rob Frohne
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
//#ifdef JACKAUDIO


#if !defined __JACKIO_H__
#define __JACKIO_H__
#include <jack/jack.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include "softrock.h"
#include "receiver.h"





int init_jack_audio(void);
void jack_cleanup(void);
int process(jack_nframes_t number_of_frames, void *arg);
void jack_shutdown (void *arg);

/* global jack variables. */
jack_client_t *softrock_client;
jack_port_t *audio_input_port_left[MAX_RECEIVERS], *audio_input_port_right[MAX_RECEIVERS];

//#endif

#endif