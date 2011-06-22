/*
 * File:   Bookmark.cpp
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

#include "Bookmark.h"

Bookmark::Bookmark() {

}

void Bookmark::setTitle(QString t) {
    title=t;
}

void Bookmark::setBand(int b) {
    band=b;
}

    void Bookmark::setFrequency(long long f) {
        frequency=f;
    }

    void Bookmark::setMode(int m) {
        mode=m;
    }

    void Bookmark::setFilter(int f) {
        filter=f;
    }

    /*

    void Bookmark::setStep(int s) {
        step=s;
    }

    void Bookmark::setSpectrumHigh(int high) {
        spectrumHigh=high;
    }

    void Bookmark::setSpectrumLow(int low) {
        spectrumLow=low;
    }

    void Bookmark::setWaterfallHigh(int high) {
        waterfallHigh=high;
    }

    void Bookmark::setWaterfallLow(int low) {
        waterfallLow=low;
    }

    */

    QString Bookmark::getTitle() {
        return title;
    }

    int Bookmark::getBand() {
        return band;
    }

    long long Bookmark::getFrequency() {
        return frequency;
    }

    int Bookmark::getMode() {
        return mode;
    }

    int Bookmark::getFilter() {
        return filter;
    }

    /*
    int Bookmark::getStep() {
        return step;
    }

    int Bookmark::getSpectrumHigh() {
        return spectrumHigh;
    }

    int Bookmark::getSpectrumLow() {
        return spectrumLow;
    }

    int Bookmark::getWaterfallHigh() {
        return waterfallHigh;
    }

    int Bookmark::getWaterfallLow() {
        return waterfallLow;
    }
    */

