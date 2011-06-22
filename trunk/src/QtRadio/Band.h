/* 
 * File:   Band.h
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 13 August 2010, 14:52
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

#ifndef BAND_H
#define	BAND_H

#include <QAction>
#include <QObject>
#include <QSettings>
#include <QDebug>

#include "BandStackEntry.h"
#include "BandLimit.h"


#define BAND_160 0
#define BAND_80  1
#define BAND_60  2
#define BAND_40  3
#define BAND_30  4
#define BAND_20  5
#define BAND_17  6
#define BAND_15  7
#define BAND_12  8
#define BAND_10  9
#define BAND_6   10
#define BAND_GEN 11
#define BAND_WWV 12
#define BAND_LAST 13
#define BAND_XVTR BAND_LAST

#define BANDSTACK_ENTRIES 5

class Band : public QObject {
    Q_OBJECT
public:
    Band();
    virtual ~Band();
    void initBand(int b);
    void selectBand(int b);
    long long bandSelected(int b,long long currentFrequency);
    int getBandStackEntry();
    void setFrequency(long long f);
    long long getFrequency();
    int getBand();
    QString getStringBand();
    QString getStringBand(int band);
    int getMode();
    int getFilter();
    int getStep();
    int getSpectrumHigh();
    int getSpectrumLow();
    int getWaterfallHigh();
    int getWaterfallLow();
    void setMode(int m);
    void setFilter(int f);
    void setSpectrumHigh(int h);
    void setSpectrumLow(int l);
    void setWaterfallHigh(int h);
    void setWaterfallLow(int l);
    void loadSettings(QSettings* settings);
    void saveSettings(QSettings* settings);
    BandLimit getBandLimits(long long min, long long max);

signals:
    void bandChanged(int previousBand,int newBand);

private:
    int currentBand;
    int currentStack;
    int stack[BAND_LAST];
    BandStackEntry bandstack[BAND_LAST][BANDSTACK_ENTRIES];
    QVector <BandLimit> limits;
};

#endif	/* BAND_H */

