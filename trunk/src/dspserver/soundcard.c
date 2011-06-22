/** 
* @file soundcard.c
* @brief Soundcard functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
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
#include "soundcard.h"

int soundcard=HPSDR;

float multimeterCalibrationOffset=35.0f;
float displayCalibrationOffset=0.0f;

/* --------------------------------------------------------------------------*/
/** 
* @brief Set soundcard
* 
* @param card
*/
void setSoundcard(int card) {
fprintf(stderr,"setSoundcard: %d\n",card);
    soundcard=card;
    switch(soundcard) {
        case AUDIGY_2_ZS:
            multimeterCalibrationOffset=1.024933f;
            displayCalibrationOffset=-29.20928f;
            break;
        case MP3_PLUS:
            multimeterCalibrationOffset=-33.40224f;
            displayCalibrationOffset=-62.84578f;
            break;
        case EXTIGY:
            multimeterCalibrationOffset=-29.30501f;
            displayCalibrationOffset=-62.099f;
            break;
        case DELTA_44:
            multimeterCalibrationOffset=-25.13887f;
            displayCalibrationOffset=-57.467f;
            break;
        case FIREBOX:
            multimeterCalibrationOffset=-20.94611f;
            displayCalibrationOffset=-54.019f;
            break;
        case EDIROL_FA_66:
            multimeterCalibrationOffset=-46.82864f;
            displayCalibrationOffset=-80.429f;
            break;
        case UNSUPPORTED_CARD:
            multimeterCalibrationOffset=-52.43533f;
            displayCalibrationOffset=-82.62103f;
            break;
        case HPSDR:
            //multimeterCalibrationOffset=-11.0f;
            //displayCalibrationOffset=-25.0f;
            multimeterCalibrationOffset=-41.0f;
            displayCalibrationOffset=-48.0f;
            break;
    }

    fprintf(stderr,"setSoundcard %f %f\n",multimeterCalibrationOffset,displayCalibrationOffset);

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Get soundcard
* 
* @param name
* 
* @return 
*/
int getSoundcardId(char* name) {
    int id=UNSUPPORTED_CARD;
    if(strcmp(name,"SANTA_CRUZ")==0) {
        id=SANTA_CRUZ;
    } else if(strcmp(name,"AUDIGY_2_ZS")==0) {
        id=AUDIGY_2_ZS;
    } else if(strcmp(name,"MP3_PLUS")==0) {
        id=MP3_PLUS;
    } else if(strcmp(name,"EXTIGY")==0) {
        id=EXTIGY;
    } else if(strcmp(name,"DELTA_44")==0) {
        id=DELTA_44;
    } else if(strcmp(name,"FIREBOX")==0) {
        id=FIREBOX;
    } else if(strcmp(name,"EDIROL_FA_66")==0) {
        id=EDIROL_FA_66;
    } else if(strcmp(name,"HPSDR")==0) {
        id=HPSDR;
    }
fprintf(stderr,"getSoundcardId: %s id=%d\n",name,id);
    return id;
}
