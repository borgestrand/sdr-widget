/** 
* @file meter_update.c
* @brief Meter update functions
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

#include <gtk/gtk.h>

#include "main.h"
#include "dttsp.h"
#include "meter.h"
#include "meter_update.h"

int updatingMeter;
int meterUpdatesPerSecond;

gint meterUpdate(gpointer data);
void meterUpdateSamples(char* command);

float meterBuffer[METER_BUFFER_SIZE];

gint timerId;

/* --------------------------------------------------------------------------*/
/** 
* @brief New meter update
*/
void newMeterUpdate() {
    timerId=gtk_timeout_add(1000/meterUpdatesPerSecond,meterUpdate,NULL);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Meter update 
* 
* @param data
* 
* @return 
*/
gint meterUpdate(gpointer data) {
    switch(meterMode) {
        case meterSIGNAL:
            meterUpdateSamples("reqMeter 1");
            break;
        case meterOFF:
            updatingMeter=FALSE;
            break;
    }
    return TRUE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Meter update samples 
* 
* @param command
*/
void meterUpdateSamples(char* command) {
    int n;

    float m=CalculateRXMeter(0,0,0);
    updateMeter(m);
}
