/** 
* @file spectrum.h
* @brief Header files for the spectrum functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// spectrum.h

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


#define spectrumSPECTRUM 0
#define spectrumPANADAPTER 1
#define spectrumSCOPE 2
#define spectrumPHASE 3
#define spectrumPHASE2 4
#define spectrumPANWATER 5
#define spectrumHISTOGRAM 6
#define spectrumNONE 7

#define spectrumWIDTH 960
#define spectrumHEIGHT 400

int spectrumMAX;
int spectrumMIN;
int spectrumSTEP;

float waterfallHighThreshold;
float waterfallLowThreshold;

int spectrumMode;
gboolean spectrumAverage;
float spectrumAverageSmoothing;

GtkWidget* newSpectrumDisplay();
void setSpectrumMode(int mode);
void updateSpectrum(float* samples);

