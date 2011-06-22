/** 
* @file spectrum_update.c
* @brief Spectrum update functions
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
#include "spectrum.h"
#include "spectrum_update.h"
#include "transmit.h"
#include "ozy.h"

int updatingSpectrum;
int spectrumUpdatesPerSecond;

gint spectrumUpdate(gpointer data);
void updateSamples();

float spectrumBuffer[SPECTRUM_BUFFER_SIZE];

gint spectrumTimerId;

/* --------------------------------------------------------------------------*/
/** 
* @brief New spectrum update
*/
void newSpectrumUpdate() {
    spectrumTimerId=gtk_timeout_add(1000/spectrumUpdatesPerSecond,spectrumUpdate,NULL);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Stop spectrum update
*/
void stopSpectrumUpdate() {
    gtk_timeout_remove(spectrumTimerId);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief set spectrum update rate
*/
void setSpectrumUpdateRate(int rate) {
    stopSpectrumUpdate();
    spectrumUpdatesPerSecond=rate;
    newSpectrumUpdate();
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum update 
* 
* @param data
* 
* @return 
*/
gint spectrumUpdate(gpointer data) {
    switch(spectrumMode) {
        case spectrumNONE:
            updatingSpectrum=FALSE;
            break;
        default:
            updateSamples();
            break;
    }
    return TRUE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Update samples
*/
void updateSamples() {
    int thread;
    thread=0;
    if(mox && !fullDuplex) thread=1;

    switch(spectrumMode) {
        case spectrumSPECTRUM:
            Process_Spectrum(thread,spectrumBuffer);
            updateSpectrum(spectrumBuffer);
            break;
        case spectrumPANADAPTER:
            Process_Panadapter(thread,spectrumBuffer);
            updateSpectrum(spectrumBuffer);
            break;
        case spectrumSCOPE:
            Process_Scope(thread,spectrumBuffer,4096);
            updateSpectrum(spectrumBuffer);
            break;
        case spectrumPHASE:
            Process_Scope(thread,spectrumBuffer,100);
            updateSpectrum(spectrumBuffer);
            break;
        case spectrumPHASE2:
            Process_Scope(thread,spectrumBuffer,100);
            updateSpectrum(spectrumBuffer);
            break;
        case spectrumPANWATER:
            Process_Panadapter(thread,spectrumBuffer);
            updateSpectrum(spectrumBuffer);
            break;
        case spectrumHISTOGRAM:
            Process_Panadapter(thread,spectrumBuffer);
            updateSpectrum(spectrumBuffer);
            break;
        case spectrumNONE:
            break;
    }

}
