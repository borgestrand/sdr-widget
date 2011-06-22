/** 
* @file soundcard.h
* @brief Header files for soundcard functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// soundcard.h

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

#define SANTA_CRUZ 0
#define AUDIGY_2_ZS 1
#define MP3_PLUS 2
#define EXTIGY 3
#define DELTA_44 4
#define FIREBOX 5
#define EDIROL_FA_66 6
#define UNSUPPORTED_CARD 7
#define HPSDR 8

char soundCardName[80];

int soundcard;
int sampleRate;

float multimeterCalibrationOffset;
float displayCalibrationOffset;

int getSoundcardId();

