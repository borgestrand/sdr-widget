/* 
 * File:   BandStackEntry.cpp
 * Author: John Melton, G0ORX/N6LYT
 * 
 * Created on 13 August 2010, 16:47
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

#include "BandStackEntry.h"

BandStackEntry::BandStackEntry() {
}

BandStackEntry::~BandStackEntry() {
}

void BandStackEntry::setFrequency(long long f) {
    frequency=f;
}

void BandStackEntry::setMode(int m) {
    mode=m;
}

void BandStackEntry::setFilter(int f) {
    filter=f;
}

void BandStackEntry::setStep(int s) {
    step=s;
}

void BandStackEntry::setSpectrumHigh(int high) {
    spectrumHigh=high;
}

void BandStackEntry::setSpectrumLow(int low) {
    spectrumLow=low;
}

void BandStackEntry::setWaterfallHigh(int high) {
    waterfallHigh=high;
}

void BandStackEntry::setWaterfallLow(int low) {
    waterfallLow=low;
}


long long BandStackEntry::getFrequency() {
    return frequency;
}

int BandStackEntry::getMode() {
    return mode;
}

int BandStackEntry::getFilter() {
    return filter;
}

int BandStackEntry::getStep() {
    return step;
}

int BandStackEntry::getSpectrumHigh() {
    return spectrumHigh;
}

int BandStackEntry::getSpectrumLow() {
    return spectrumLow;
}

int BandStackEntry::getWaterfallHigh() {
    return waterfallHigh;
}

int BandStackEntry::getWaterfallLow() {
    return waterfallLow;
}
