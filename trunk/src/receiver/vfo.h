/** 
* @file vfo.h
* @brief Header files for the vfo functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// vfo.h

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

long long frequencyA;
long long frequencyB;
int frequencyAChanged;
int frequencyBChanged;

long long frequencyALO;
long long frequencyBLO;
long long ddsAFrequency;
long long ddsBFrequency;

long long frequencyMin;
long long frequencyMax;

long frequencyIncrement;

int bSubRx;
int bSplit;
int splitChanged;

void vfoSaveState();
void vfoRestoreState();

GtkWidget* buildVfoUI();
void vfoIncrementFrequency(long increment);
void setAFrequency(long long *frequency);
void setBFrequency(long long frequency);
void setLOFrequency(long long frequency);

void vfoTransmit(gpointer data);
int vfoStepFrequency(gpointer data);

void vfoRX2(int state);
void vfoSplit(int state);
