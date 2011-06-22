/**
* @file sdr1000.h
* @brief sdr1000 audio implementation
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

#define PORTAUDIO

#ifdef __cplusplus
 extern "C" {
#endif

extern int sdr1000_init();
extern int create_sdr1000_thread();
extern void sdr1000_set_device(char* d);
extern char* sdr1000_get_device();
extern void sdr1000_set_receivers(int r);
extern int sdr1000_get_receivers();
extern void sdr1000_set_sample_rate(int r);
extern int sdr1000_get_sample_rate();

extern void sdr1000_set_input(char* d);
extern char* sdr1000_get_input();
extern void sdr1000_set_output(char* d);
extern char* sdr1000_get_output();

#ifdef __cplusplus
 }
#endif

