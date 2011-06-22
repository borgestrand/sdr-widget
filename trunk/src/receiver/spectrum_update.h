/** 
* @file spectrum_update.h
* @brief Header files for the spectrum update functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// spectrum_update.h

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

#define SPEC_SEMI_RAW 0
#define SPEC_PRE_FILT 1
#define SPEC_POST_FILT 2
#define SPEC_POST_AGC 3
#define SPEC_POST_DET 4
#define SPEC_PREMOD 4

#define SPEC_MAG 0
#define SPEC_PWR 1

#define SPECTRUM_BUFFER_SIZE 4096
#define SPECTRUM_UPDATES_PER_SECOND 15

int spectrumUpdatesPerSecond;

void newSpectrumUpdate();
void stopSpectrumUpdate();
void setSpectrumUpdateRate(int rate);
