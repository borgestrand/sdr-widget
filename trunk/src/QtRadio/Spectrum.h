   /*
 * File:   Spectrum.h
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 16 August 2010, 10:03
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

#ifndef SPECTRUM_H
#define	SPECTRUM_H

#include <QtCore>

#include <QFrame>
#include <QPainter>
#include <QMouseEvent>
//#include <QGLWidget>

#include <Meter.h>

class Spectrum: public QFrame {
    Q_OBJECT
public:
    Spectrum();
    Spectrum(QWidget*& widget);
    virtual ~Spectrum();
    void setObjectName(QString name);
    void setGeometry(QRect rect);
    void initialize();
    void setSampleRate(int r);
    void setFrequency(long long f);
    void setSubRxFrequency(long long f);
    void setFilter(int low,int high);
    void updateSpectrum(char* header,char* buffer,int width);
    int samplerate();

    int getHigh();
    int getLow();
    void setHigh(int high);
    void setLow(int low);

    void setSubRxState(bool state);

    void setMode(QString m);
    void setBand(QString b);
    void setFilter(QString f);

    void setHost(QString h);
    void setReceiver(int r);

    void setBandLimits(long long min,long long max);

signals:
    void frequencyMoved(int steps,int step);
    void frequencyChanged(long long frequency);
    void spectrumHighChanged(int high);
    void spectrumLowChanged(int low);
    void waterfallHighChanged(int high);
    void waterfallLowChanged(int low);

protected:
    void paintEvent(QPaintEvent*);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    /*
    void mouseDoubleClickEvent ( QMouseEvent * event );
    */

    void wheelEvent(QWheelEvent *event);

private:
    float* samples;
    int spectrumHigh;
    int spectrumLow;

    QString host;
    int receiver;
    QString band;
    QString mode;
    QString filter;

    int button;
    int startX;
    int lastX;
    int moved;

    int sampleRate;
    int meter;
    int maxMeter;
    int meterCount;
    int subrx_meter;
    int subrx_maxMeter;
    int subrx_meterCount;

    int filterLow;
    int filterHigh;
    long long frequency;
    QString strFrequency;
    long long subRxFrequency;
    QString strSubRxFrequency;
    bool subRx;

    long long band_min;
    long long band_max;

    QVector <QPoint> plot;

    Meter* sMeterMain;
    Meter* sMeterSub;
};


#endif	/* SPECTRUM_H */

