/* 
 * File:   Mode.cpp
 * Author: John Melton, G0ORX/N6LYT
 * 
 * Created on 15 August 2010, 07:46
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


#include "Mode.h"

Mode::Mode() {
    currentMode=MODE_LSB;
}

Mode::~Mode() {
}

void Mode::setMode(int m) {
    int previousMode=currentMode;
    currentMode=m;
    emit modeChanged(previousMode,currentMode);
}

int Mode::getMode() {
    return currentMode;
}

QString Mode::getStringMode() {
    return getStringMode(currentMode);
}

QString Mode::getStringMode(int mode) {
    QString m="INV";

    switch(mode) {
        case MODE_LSB:
            m="LSB";
            break;
        case MODE_USB:
            m="USB";
            break;
        case MODE_DSB:
            m="DSB";
            break;
        case MODE_CWL:
            m="CWL";
            break;
        case MODE_CWU:
            m="CWU";
            break;
        case MODE_FMN:
            m="FMN";
            break;
        case MODE_AM:
            m="AM";
            break;
        case MODE_DIGU:
            m="DIGU";
            break;
        case MODE_SPEC:
            m="SPEC";
            break;
        case MODE_DIGL:
            m="DIGL";
            break;
        case MODE_SAM:
            m="SAM";
            break;
        case MODE_DRM:
            m="DRM";
            break;
    }

    return m;
}
