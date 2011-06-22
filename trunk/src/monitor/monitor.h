/**
* @file monitor.h
* @brief simple spectrum display to monitor a receiver
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

#define BUFFER_SIZE 1024

int get_receiver();
void set_receiver(int r);
float get_waterfall_high();
void set_waterfall_high(float high);
float get_waterfall_low();
void set_waterfall_low(float low);
int get_update_fps();
void set_update_fps(int update);
long get_frequency();
void set_frequency(long f);

