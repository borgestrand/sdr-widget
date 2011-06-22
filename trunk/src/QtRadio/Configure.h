/* 
 * File:   Configure.h
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 16 August 2010, 20:03
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

#ifndef _CONFIGURE_H
#define	_CONFIGURE_H

#include <QSettings>
#include <QDebug>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioDeviceInfo>

#include "ui_Configure.h"

#include "Audio.h"
#include "Xvtr.h"

class Configure : public QDialog {
    Q_OBJECT
public:
    Configure();
    virtual ~Configure();
    void initAudioDevices(Audio* audio);
    void initXvtr(Xvtr* xvtr);
    void loadSettings(QSettings* settings);
    void saveSettings(QSettings* settings);

    void connected(bool state);
    void updateXvtrList(Xvtr* xvtr);

    QString getHost();
    int getReceiver();
    
    int getSpectrumHigh();
    int getSpectrumLow();
    int getFps();

    void setSpectrumHigh(int high);
    void setSpectrumLow(int low);

    int getWaterfallHigh();
    int getWaterfallLow();

    void setWaterfallHigh(int high);
    void setWaterfallLow(int low);

    QAudioFormat::Endian getByteOrder();
    int getSampleRate();
    int getChannels();
    
    int getNrTaps();
    int getNrDelay();
    double getNrGain();
    double getNrLeak();

    int getAnfTaps();
    int getAnfDelay();
    double getAnfGain();
    double getAnfLeak();

    double getNbThreshold();
    double getSdromThreshold();

signals:
    void hostChanged(QString host);
    void receiverChanged(int receiver);
    void spectrumHighChanged(int high);
    void spectrumLowChanged(int low);
    void fpsChanged(int fps);
    void waterfallHighChanged(int high);
    void waterfallLowChanged(int low);
    void audioDeviceChanged(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian order);
//    void sampleRateChanged(int rate);
//    void channelsChanged(int channels);
//    void byteOrderChanged(QAudioFormat::Endian order);

//    void nrTapsChanged(int taps);
//    void nrDelayChanged(int delay);
//    void nrGainChanged(double gain);
//    void nrLeakChanged(double leak);
    void nrValuesChanged(int taps,int delay,double gain,double leak);

//    void anfTapsChanged(int taps);
//    void anfDelayChanged(int delay);
//    void anfGainChanged(double gain);
//    void anfLeakChanged(double leak);
    void anfValuesChanged(int taps,int delay,double gain,double leak);

    void nbThresholdChanged(double threshold);
    void sdromThresholdChanged(double threshold);

    void addXVTR(QString title,long long minFrequency,long long maxFrequency,long long ifFrequency,long long freq,int m,int filt);
    void deleteXVTR(int index);


public slots:
    void slotHostChanged(int selection);
    void slotReceiverChanged(int receiver);
    void slotSpectrumHighChanged(int high);
    void slotSpectrumLowChanged(int low);
    void slotFpsChanged(int fps);
    void slotWaterfallHighChanged(int high);
    void slotWaterfallLowChanged(int low);
    void slotAudioDeviceChanged(int selection);
    void slotSampleRateChanged(int rate);
    void slotChannelsChanged(int channels);
    void slotByteOrderChanged(int selection);

    void slotNrTapsChanged(int taps);
    void slotNrDelayChanged(int delay);
    void slotNrGainChanged(int gain);
    void slotNrLeakChanged(int leak);

    void slotAnfTapsChanged(int taps);
    void slotAnfDelayChanged(int delay);
    void slotAnfGainChanged(int gain);
    void slotAnfLeakChanged(int leak);

    void slotNbThresholdChanged(int threshold);
    void slotSdromThresholdChanged(int threshold);

    void slotXVTRAdd();
    void slotXVTRDelete();

private:
    Ui::Configure widget;
};

#endif	/* _CONFIGURE_H */
