/*
 * File:   Bppkmark.h
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

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QObject>

class Bookmark : public QObject {
    Q_OBJECT

public:
    Bookmark();
    void setTitle(QString title);
    void setBand(int b);
    void setFrequency(long long f);
    void setMode(int m);
    void setFilter(int f);
    void setStep(int s);
    void setSpectrumHigh(int high);
    void setSpectrumLow(int low);
    void setWaterfallHigh(int high);
    void setWaterfallLow(int low);

    QString getTitle();
    int getBand();
    long long getFrequency();
    int getMode();
    int getFilter();
    int getStep();
    int getSpectrumHigh();
    int getSpectrumLow();
    int getWaterfallHigh();
    int getWaterfallLow();


private:
    QString title;
    int band;
    long long frequency;
    int mode;
    int filter;
    int step;
    int spectrumHigh;
    int spectrumLow;
    int spectrumStep;
    int waterfallHigh;
    int waterfallLow;
};

#endif // BOOKMARK_H

