/*
 * File:   UI.cpp
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 13 August 2010, 14:28
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

#include <QDebug>
#include <QSettings>

#include "UI.h"
#include "About.h"
#include "Configure.h"
#include "Band.h"
#include "Mode.h"
#include "FiltersBase.h"
#include "CWLFilters.h"
#include "CWUFilters.h"
#include "LSBFilters.h"
#include "USBFilters.h"
#include "DSBFilters.h"
#include "AMFilters.h"
#include "SAMFilters.h"
#include "FMNFilters.h"
#include "DIGLFilters.h"
#include "DIGUFilters.h"
#include "XvtrEntry.h"

UI::UI() {
    widget.setupUi(this);

    widget.gridLayout->setContentsMargins(2,2,2,2);
    widget.gridLayout->setVerticalSpacing(0);

    // connect up all the menus
    connect(widget.actionAbout,SIGNAL(triggered()),this,SLOT(actionAbout()));
    connect(widget.actionConnectToServer,SIGNAL(triggered()),this,SLOT(actionConnect()));
    connect(widget.actionDisconnectFromServer,SIGNAL(triggered()),this,SLOT(actionDisconnect()));

    connect(widget.actionSubrx,SIGNAL(triggered()),this,SLOT(actionSubRx()));
    connect(widget.actionBandscope,SIGNAL(triggered()),this,SLOT(actionBandscope()));
    connect(widget.actionRecord,SIGNAL(triggered()),this,SLOT(actionRecord()));

    connect(&connection,SIGNAL(isConnected()),this,SLOT(connected()));
    connect(&connection,SIGNAL(disconnected(QString)),this,SLOT(disconnected(QString)));
    connect(&connection,SIGNAL(audioBuffer(char*,char*)),this,SLOT(audioBuffer(char*,char*)));
    connect(&connection,SIGNAL(spectrumBuffer(char*,char*)),this,SLOT(spectrumBuffer(char*,char*)));

    connect(widget.actionConfig,SIGNAL(triggered()),this,SLOT(actionConfigure()));

    connect(widget.actionMuteMainRx,SIGNAL(triggered()),this,SLOT(actionMuteMainRx()));
    connect(widget.actionMuteSubRx,SIGNAL(triggered()),this,SLOT(actionMuteSubRx()));

    connect(widget.actionGain_10,SIGNAL(triggered()),this,SLOT(actionGain_10()));
    connect(widget.actionGain_20,SIGNAL(triggered()),this,SLOT(actionGain_20()));
    connect(widget.actionGain_30,SIGNAL(triggered()),this,SLOT(actionGain_30()));
    connect(widget.actionGain_40,SIGNAL(triggered()),this,SLOT(actionGain_40()));
    connect(widget.actionGain_50,SIGNAL(triggered()),this,SLOT(actionGain_50()));
    connect(widget.actionGain_60,SIGNAL(triggered()),this,SLOT(actionGain_60()));
    connect(widget.actionGain_70,SIGNAL(triggered()),this,SLOT(actionGain_70()));
    connect(widget.actionGain_80,SIGNAL(triggered()),this,SLOT(actionGain_80()));
    connect(widget.actionGain_90,SIGNAL(triggered()),this,SLOT(actionGain_90()));
    connect(widget.actionGain_100,SIGNAL(triggered()),this,SLOT(actionGain_100()));

    connect(widget.actionKeypad, SIGNAL(triggered()),this,SLOT(actionKeypad()));
    connect(&keypad,SIGNAL(setKeypadFrequency(long long)),this,SLOT(setKeypadFrequency(long long)));

    connect(widget.action160, SIGNAL(triggered()),this,SLOT(action160()));
    connect(widget.action80, SIGNAL(triggered()),this,SLOT(action80()));
    connect(widget.action60, SIGNAL(triggered()),this,SLOT(action60()));
    connect(widget.action40, SIGNAL(triggered()),this,SLOT(action40()));
    connect(widget.action30, SIGNAL(triggered()),this,SLOT(action30()));
    connect(widget.action20, SIGNAL(triggered()),this,SLOT(action20()));
    connect(widget.action17, SIGNAL(triggered()),this,SLOT(action17()));
    connect(widget.action15, SIGNAL(triggered()),this,SLOT(action15()));
    connect(widget.action12, SIGNAL(triggered()),this,SLOT(action12()));
    connect(widget.action10, SIGNAL(triggered()),this,SLOT(action10()));
    connect(widget.action6, SIGNAL(triggered()),this,SLOT(action6()));
    connect(widget.actionGen, SIGNAL(triggered()),this,SLOT(actionGen()));
    connect(widget.actionWWV, SIGNAL(triggered()),this,SLOT(actionWWV()));

    connect(widget.actionCWL,SIGNAL(triggered()),this,SLOT(actionCWL()));
    connect(widget.actionCWU,SIGNAL(triggered()),this,SLOT(actionCWU()));
    connect(widget.actionLSB,SIGNAL(triggered()),this,SLOT(actionLSB()));
    connect(widget.actionUSB,SIGNAL(triggered()),this,SLOT(actionUSB()));
    connect(widget.actionDSB,SIGNAL(triggered()),this,SLOT(actionDSB()));
    connect(widget.actionAM,SIGNAL(triggered()),this,SLOT(actionAM()));
    connect(widget.actionSAM,SIGNAL(triggered()),this,SLOT(actionSAM()));
    connect(widget.actionFMN,SIGNAL(triggered()),this,SLOT(actionFMN()));
    connect(widget.actionDIGL,SIGNAL(triggered()),this,SLOT(actionDIGL()));
    connect(widget.actionDIGU,SIGNAL(triggered()),this,SLOT(actionDIGU()));

    connect(widget.actionFilter_0,SIGNAL(triggered()),this,SLOT(actionFilter0()));
    connect(widget.actionFilter_1,SIGNAL(triggered()),this,SLOT(actionFilter1()));
    connect(widget.actionFilter_2,SIGNAL(triggered()),this,SLOT(actionFilter2()));
    connect(widget.actionFilter_3,SIGNAL(triggered()),this,SLOT(actionFilter3()));
    connect(widget.actionFilter_4,SIGNAL(triggered()),this,SLOT(actionFilter4()));
    connect(widget.actionFilter_5,SIGNAL(triggered()),this,SLOT(actionFilter5()));
    connect(widget.actionFilter_6,SIGNAL(triggered()),this,SLOT(actionFilter6()));
    connect(widget.actionFilter_7,SIGNAL(triggered()),this,SLOT(actionFilter7()));
    connect(widget.actionFilter_8,SIGNAL(triggered()),this,SLOT(actionFilter8()));
    connect(widget.actionFilter_9,SIGNAL(triggered()),this,SLOT(actionFilter9()));

    connect(widget.actionANF,SIGNAL(triggered()),this,SLOT(actionANF()));
    connect(widget.actionNR,SIGNAL(triggered()),this,SLOT(actionNR()));
    connect(widget.actionNB,SIGNAL(triggered()),this,SLOT(actionNB()));
    connect(widget.actionSDROM,SIGNAL(triggered()),this,SLOT(actionSDROM()));

    connect(widget.actionPolyphase,SIGNAL(triggered()),this,SLOT(actionPolyphase()));

    connect(widget.actionLong,SIGNAL(triggered()),this,SLOT(actionLong()));
    connect(widget.actionSlow,SIGNAL(triggered()),this,SLOT(actionSlow()));
    connect(widget.actionMedium,SIGNAL(triggered()),this,SLOT(actionMedium()));
    connect(widget.actionFast,SIGNAL(triggered()),this,SLOT(actionFast()));


    connect(widget.actionPreamp,SIGNAL(triggered()),this,SLOT(actionPreamp()));

    connect(widget.actionBookmarkThisFrequency,SIGNAL(triggered()),this,SLOT(actionBookmark()));
    connect(widget.actionEditBookmarks,SIGNAL(triggered()),this,SLOT(editBookmarks()));



    // connect up band and frequency changes
    connect(&band,SIGNAL(bandChanged(int,int)),this,SLOT(bandChanged(int,int)));
//    connect(&band,SIGNAL(frequencyChanged(long long)),this,SLOT(frequencyChanged(long long)));

    // connect up mode changes
    connect(&mode,SIGNAL(modeChanged(int,int)),this,SLOT(modeChanged(int,int)));

    // connect up filter changes
    connect(&filters,SIGNAL(filtersChanged(FiltersBase*,FiltersBase*)),this,SLOT(filtersChanged(FiltersBase*,FiltersBase*)));
    connect(&filters,SIGNAL(filterChanged(int,int)),this,SLOT(filterChanged(int,int)));

    // connect up spectrum frame
    connect(widget.spectrumFrame, SIGNAL(frequencyMoved(int,int)),
            this, SLOT(frequencyMoved(int,int)));
    connect(widget.spectrumFrame, SIGNAL(frequencyChanged(long long)),
            this, SLOT(frequencyChanged(long long)));
    connect(widget.spectrumFrame, SIGNAL(spectrumHighChanged(int)),
            this,SLOT(spectrumHighChanged(int)));
    connect(widget.spectrumFrame, SIGNAL(spectrumLowChanged(int)),
            this,SLOT(spectrumLowChanged(int)));
    connect(widget.spectrumFrame, SIGNAL(waterfallHighChanged(int)),
            this,SLOT(waterfallHighChanged(int)));
    connect(widget.spectrumFrame, SIGNAL(waterfallLowChanged(int)),
            this,SLOT(waterfallLowChanged(int)));

    // connect up waterfall frame
    connect(widget.waterfallFrame, SIGNAL(frequencyMoved(int,int)),
            this, SLOT(frequencyMoved(int,int)));

    // connect up configuration changes
    connect(&configure,SIGNAL(spectrumHighChanged(int)),this,SLOT(spectrumHighChanged(int)));
    connect(&configure,SIGNAL(spectrumLowChanged(int)),this,SLOT(spectrumLowChanged(int)));
    connect(&configure,SIGNAL(fpsChanged(int)),this,SLOT(fpsChanged(int)));
    connect(&configure,SIGNAL(waterfallHighChanged(int)),this,SLOT(waterfallHighChanged(int)));
    connect(&configure,SIGNAL(waterfallLowChanged(int)),this,SLOT(waterfallLowChanged(int)));

    configure.initAudioDevices(&audio);
    connect(&configure,SIGNAL(audioDeviceChanged(QAudioDeviceInfo,int,int,QAudioFormat::Endian)),this,SLOT(audioDeviceChanged(QAudioDeviceInfo,int,int,QAudioFormat::Endian)));

    connect(&configure,SIGNAL(hostChanged(QString)),this,SLOT(hostChanged(QString)));
    connect(&configure,SIGNAL(receiverChanged(int)),this,SLOT(receiverChanged(int)));

    connect(&configure,SIGNAL(nrValuesChanged(int,int,double,double)),this,SLOT(nrValuesChanged(int,int,double,double)));
    connect(&configure,SIGNAL(anfValuesChanged(int,int,double,double)),this,SLOT(anfValuesChanged(int,int,double,double)));
    connect(&configure,SIGNAL(nbThresholdChanged(double)),this,SLOT(nbThresholdChanged(double)));
    connect(&configure,SIGNAL(sdromThresholdChanged(double)),this,SLOT(sdromThresholdChanged(double)));

    connect(&bookmarks,SIGNAL(bookmarkSelected(QAction*)),this,SLOT(selectBookmark(QAction*)));
    connect(&bookmarkDialog,SIGNAL(accepted()),this,SLOT(addBookmark()));

    connect(&configure,SIGNAL(addXVTR(QString,long long,long long,long long,long long,int,int)),this,SLOT(addXVTR(QString,long long,long long,long long,long long,int,int)));
    connect(&configure,SIGNAL(deleteXVTR(int)),this,SLOT(deleteXVTR(int)));


    connect(&xvtr,SIGNAL(xvtrSelected(QAction*)),this,SLOT(selectXVTR(QAction*)));


    bandscope=NULL;

    fps=15;
    gain=100;
    subRx=FALSE;
    subRxGain=100;
    agc=AGC_SLOW;
    cwPitch=600;

    audio_device=0;
    audio_sample_rate=configure.getSampleRate();
    audio_channels=configure.getChannels();
    audio_byte_order=configure.getByteOrder();
    audio.initialize_audio(AUDIO_BUFFER_SIZE);

    // load any saved settings
    loadSettings();
    switch(agc) {
        case AGC_SLOW:
            widget.actionSlow->setChecked(TRUE);
            break;
        case AGC_MEDIUM:
            widget.actionMedium->setChecked(TRUE);
            break;
        case AGC_FAST:
            widget.actionFast->setChecked(TRUE);
            break;
        case AGC_LONG:
            widget.actionLong->setChecked(TRUE);
            break;
    }

    fps=configure.getFps();

    configure.updateXvtrList(&xvtr);
    xvtr.buildMenu(widget.menuXVTR);

    widget.spectrumFrame->setHost(configure.getHost());
    widget.spectrumFrame->setReceiver(configure.getReceiver());

    widget.actionSubrx->setDisabled(TRUE);
    widget.actionMuteSubRx->setDisabled(TRUE);

    band.initBand(band.getBand());
    
}

UI::~UI() {
    connection.disconnect();
}

void UI::actionAbout() {
    about.setVisible(TRUE);
}

void UI::loadSettings() {
    QSettings settings("G0ORX", "QtRadio");
    qDebug() << "saveSettings: " << settings.fileName();

    band.loadSettings(&settings);
    xvtr.loadSettings(&settings);
    configure.loadSettings(&settings);
    configure.updateXvtrList(&xvtr);
    bookmarks.loadSettings(&settings);
    bookmarks.buildMenu(widget.menuView_Bookmarks);

    settings.beginGroup("UI");
    if(settings.contains("gain")) gain=subRxGain=settings.value("gain").toInt();
    if(settings.contains("agc")) agc=settings.value("agc").toInt();
    settings.endGroup();
}

void UI::saveSettings() {
    QString s;
    Bookmark* bookmark;

    QSettings settings("G0ORX","QtRadio");

    qDebug() << "saveSettings: " << settings.fileName();

    settings.clear();

    configure.saveSettings(&settings);
    band.saveSettings(&settings);
    xvtr.saveSettings(&settings);
    bookmarks.saveSettings(&settings);

    settings.beginGroup("UI");
    settings.setValue("gain",gain);
    settings.setValue("subRxGain",subRxGain);
    settings.setValue("agc",agc);
    settings.endGroup();

}

void UI::hostChanged(QString host) {
    widget.spectrumFrame->setHost(host);
}

void UI::receiverChanged(int rx) {
    widget.spectrumFrame->setReceiver(rx);
}

void UI::closeEvent(QCloseEvent* event) {
    saveSettings();
}

void UI::actionConfigure() {
    configure.show();
}

void UI::spectrumHighChanged(int high) {
    //qDebug() << __FUNCTION__ << ": " << high;

    widget.spectrumFrame->setHigh(high);
    configure.setSpectrumHigh(high);
    band.setSpectrumHigh(high);
}

void UI::spectrumLowChanged(int low) {
    //qDebug() << __FUNCTION__ << ": " << low;

    widget.spectrumFrame->setLow(low);
    configure.setSpectrumLow(low);
    band.setSpectrumLow(low);
}

void UI::fpsChanged(int f) {
    //qDebug() << "fpsChanged:" << f;
    fps=f;
}

void UI::waterfallHighChanged(int high) {
    //qDebug() << __LINE__ << __FUNCTION__ << ": " << high;

    widget.waterfallFrame->setHigh(high);
    configure.setWaterfallHigh(high);
    band.setWaterfallHigh(high);
}

void UI::waterfallLowChanged(int low) {
    //qDebug() << __FUNCTION__ << ": " << low;

    widget.waterfallFrame->setLow(low);
    configure.setWaterfallLow(low);
    band.setWaterfallLow(low);
}

void UI::audioDeviceChanged(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder) {
    audio.select_audio(info,rate,channels,byteOrder);
}

void UI::encodingChanged(int choice) {
    // not supported yet
}

void UI::actionConnect() {
    //qDebug() << "UI::actionConnect";
    widget.statusbar->clearMessage();
    connection.connect(configure.getHost(), DSPSERVER_BASE_PORT+configure.getReceiver());
    widget.spectrumFrame->setHost(configure.getHost());
    widget.spectrumFrame->setReceiver(configure.getReceiver());
}

void UI::actionDisconnect() {
    //qDebug() << "UI::actionDisconnect";

//    qTimer->stop();

    connection.disconnect();
    widget.actionConnectToServer->setDisabled(FALSE);
    widget.actionDisconnectFromServer->setDisabled(TRUE);
    widget.actionSubrx->setDisabled(TRUE);
    widget.actionMuteSubRx->setDisabled(TRUE);

    configure.connected(FALSE);
}

void UI::connected() {
    QString command;

    qDebug() << "UI::connected";

    configure.connected(TRUE);

    // send initial settings
    frequency=band.getFrequency();
    command.clear(); QTextStream(&command) << "setFrequency " << frequency;
    connection.sendCommand(command);
    widget.spectrumFrame->setFrequency(frequency);
    widget.waterfallFrame->setFrequency(frequency);

    command.clear(); QTextStream(&command) << "setMode " << band.getMode();
    connection.sendCommand(command);

    int low,high;
    if(mode.getMode()==MODE_CWL) {
        low=-cwPitch-filters.getLow();
        high=-cwPitch+filters.getHigh();
    } else if(mode.getMode()==MODE_CWU) {
        low=cwPitch-filters.getLow();
        high=cwPitch+filters.getHigh();
    } else {
        low=filters.getLow();
        high=filters.getHigh();
    }
    command.clear(); QTextStream(&command) << "setFilter " << low << " " << high;
    connection.sendCommand(command);

    qDebug() << "connected calling widget.spectrumFrame.setFilter";

    widget.spectrumFrame->setFilter(low,high);
    widget.waterfallFrame->setFilter(low,high);

    widget.actionConnectToServer->setDisabled(TRUE);
    widget.actionDisconnectFromServer->setDisabled(FALSE);
    widget.actionSubrx->setDisabled(FALSE);
    widget.actionMuteSubRx->setDisabled(TRUE);


    // start the audio
    audio_buffers=0;
    actionGain(gain);
    connection.sendCommand(command);

    if (!getenv("QT_RADIO_NO_LOCAL_AUDIO")) {
       command.clear(); QTextStream(&command) << "startAudioStream " << (400*(audio.get_sample_rate()/8000)) << " " << audio.get_sample_rate() << " " << audio.get_channels();
       connection.sendCommand(command);
    }

    command.clear(); QTextStream(&command) << "SetPan 0.5"; // center
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetANFVals " << configure.getAnfTaps() << " " << configure.getAnfDelay() << " "
                                           << configure.getAnfGain() << " " << configure.getAnfLeak();
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetNRVals " << configure.getNrTaps() << " " << configure.getNrDelay() << " "
                                           << configure.getNrGain() << " " << configure.getNrLeak();
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetNBVals " << configure.getNbThreshold();
    connection.sendCommand(command);

    command.clear(); QTextStream(&command) << "SetANF " << (widget.actionANF->isChecked()?"true":"false");
    connection.sendCommand(command);
    command.clear(); QTextStream(&command) << "SetNR " << (widget.actionNR->isChecked()?"true":"false");
    connection.sendCommand(command);
    command.clear(); QTextStream(&command) << "SetNB " << (widget.actionNB->isChecked()?"true":"false");
    connection.sendCommand(command);

    //command.clear(); QTextStream(&command) << "SetDCBlock 1";
    //connection.sendCommand(command);

    // start the spectrum
    //qDebug() << "starting spectrum timer";
    //QTimer::singleShot(1000/fps,this,SLOT(updateSpectrum()));
    updateSpectrum();
}

void UI::disconnected(QString message) {
    qDebug() << "UI::disconnected: " << message;

    widget.statusbar->showMessage(message,0);

    widget.actionConnectToServer->setDisabled(FALSE);
    widget.actionDisconnectFromServer->setDisabled(TRUE);
    widget.actionSubrx->setDisabled(TRUE);
    widget.actionMuteSubRx->setDisabled(TRUE);

    configure.connected(FALSE);
}

void UI::updateSpectrum() {
    QString command;
    command.clear(); QTextStream(&command) << "getSpectrum " << widget.spectrumFrame->width();
    connection.sendCommand(command);
}

void UI::spectrumBuffer(char* header,char* buffer) {
    //qDebug() << "spectrumBuffer";
    int length=atoi(&header[26]);
    sampleRate=atoi(&header[32]);
    widget.spectrumFrame->updateSpectrum(header,buffer,length);
    widget.waterfallFrame->updateWaterfall(header,buffer,length);
    connection.freeBuffers(header,buffer);
    QTimer::singleShot(1000/fps,this,SLOT(updateSpectrum()));
}

void UI::audioBuffer(char* header,char* buffer) {
    //qDebug() << "audioBuffer";
    int length=atoi(&header[26]);
    if(audio_buffers==0) {
        first_audio_header=header;
        first_audio_buffer=buffer;
        audio_buffers++;
    } else if(audio_buffers==1) {
        audio_buffers++;
        audio.process_audio(first_audio_header,first_audio_buffer,length);
        connection.freeBuffers(first_audio_header,first_audio_buffer);
        audio.process_audio(header,buffer,length);
        connection.freeBuffers(header,buffer);
    } else {
        audio.process_audio(header,buffer,length);
        connection.freeBuffers(header,buffer);
    }
}

void UI::actionSubRx() {
    QString command;
    if(subRx) {
        // on, so turn off
        subRx=FALSE;
        widget.spectrumFrame->setSubRxState(FALSE);
        widget.waterfallFrame->setSubRxState(FALSE);
        widget.actionMuteSubRx->setChecked(FALSE);
        widget.actionMuteSubRx->setDisabled(TRUE);
    } else {
        subRx=TRUE;

        // check frequency in range
        int samplerate = widget.spectrumFrame->samplerate();
        long long frequency=band.getFrequency();
        if ((subRxFrequency < (frequency - (samplerate / 2))) || (subRxFrequency > (frequency + (samplerate / 2)))) {
            subRxFrequency=band.getFrequency();
        }
        widget.spectrumFrame->setSubRxState(TRUE);
        widget.waterfallFrame->setSubRxState(TRUE);
        command.clear(); QTextStream(&command) << "SetSubRXFrequency " << frequency - subRxFrequency;
        connection.sendCommand(command);

        setSubRxPan();

        widget.actionMuteSubRx->setDisabled(FALSE);
    }

    //widget.actionSubrx.setChecked(subRx);
    command.clear(); QTextStream(&command) << "SetSubRX " << subRx;
    connection.sendCommand(command);

}

void UI::setSubRxGain(int gain) {
    QString command;
    subRxGain=gain;
    command.clear(); QTextStream(&command) << "SetSubRXOutputGain " << subRxGain;
    connection.sendCommand(command);

    qDebug() << command;
}

void UI::actionKeypad() {
    keypad.clear();
    keypad.show();
}

void UI::setKeypadFrequency(long long f) {
    frequencyChanged(f);
}

void UI::action160() {
    band.selectBand(BAND_160);
}

void UI::action80() {
    band.selectBand(BAND_80);
}

void UI::action60() {
    band.selectBand(BAND_60);
}

void UI::action40() {
    band.selectBand(BAND_40);
}

void UI::action30() {
    band.selectBand(BAND_30);
}

void UI::action20() {
    band.selectBand(BAND_20);
}

void UI::action17() {
    band.selectBand(BAND_17);
}

void UI::action15() {
    band.selectBand(BAND_15);
}

void UI::action12() {
    band.selectBand(BAND_12);
}

void UI::action10() {
    band.selectBand(BAND_10);
}

void UI::action6() {
    band.selectBand(BAND_6);
}

void UI::actionGen() {
    band.selectBand(BAND_GEN);
}

void UI::actionWWV() {
    band.selectBand(BAND_WWV);
}

void UI::bandChanged(int previousBand,int newBand) {
    qDebug() << "UI::bandChanged: " << previousBand << "," << newBand;
    // uncheck previous band
    switch(previousBand) {
        case BAND_160:
            widget.action160->setChecked(FALSE);
            break;
        case BAND_80:
            widget.action80->setChecked(FALSE);
            break;
        case BAND_60:
            widget.action60->setChecked(FALSE);
            break;
        case BAND_40:
            widget.action40->setChecked(FALSE);
            break;
        case BAND_30:
            widget.action30->setChecked(FALSE);
            break;
        case BAND_20:
            widget.action20->setChecked(FALSE);
            break;
        case BAND_17:
            widget.action17->setChecked(FALSE);
            break;
        case BAND_15:
            widget.action15->setChecked(FALSE);
            break;
        case BAND_12:
            widget.action12->setChecked(FALSE);
            break;
        case BAND_10:
            widget.action10->setChecked(FALSE);
            break;
        case BAND_6:
            widget.action6->setChecked(FALSE);
            break;
        case BAND_GEN:
            widget.actionGen->setChecked(FALSE);
            break;
        case BAND_WWV:
            widget.actionWWV->setChecked(FALSE);
            break;
    }

    // check new band
    switch(newBand) {
        case BAND_160:
            widget.action160->setChecked(TRUE);
            break;
        case BAND_80:
            widget.action80->setChecked(TRUE);
            break;
        case BAND_60:
            widget.action60->setChecked(TRUE);
            break;
        case BAND_40:
            widget.action40->setChecked(TRUE);
            break;
        case BAND_30:
            widget.action30->setChecked(TRUE);
            break;
        case BAND_20:
            widget.action20->setChecked(TRUE);
            break;
        case BAND_17:
            widget.action17->setChecked(TRUE);
            break;
        case BAND_15:
            widget.action15->setChecked(TRUE);
            break;
        case BAND_12:
            widget.action12->setChecked(TRUE);
            break;
        case BAND_10:
            widget.action10->setChecked(TRUE);
            break;
        case BAND_6:
            widget.action6->setChecked(TRUE);
            break;
        case BAND_GEN:
            widget.actionGen->setChecked(TRUE);
            break;
        case BAND_WWV:
            widget.actionWWV->setChecked(TRUE);
            break;
    }

    // get the band setting
    mode.setMode(band.getMode());
    frequency=band.getFrequency();
    int samplerate = widget.spectrumFrame->samplerate();
    if(subRx) {
        if ((subRxFrequency < (frequency - (samplerate / 2))) || (subRxFrequency > (frequency + (samplerate / 2)))) {
            subRxFrequency=frequency;
        }
    }

    QString command;
    command.clear(); QTextStream(&command) << "setFrequency " << frequency;
    connection.sendCommand(command);

    widget.spectrumFrame->setFrequency(frequency);
    widget.spectrumFrame->setSubRxFrequency(subRxFrequency);
    widget.spectrumFrame->setHigh(band.getSpectrumHigh());
    widget.spectrumFrame->setLow(band.getSpectrumLow());
    widget.waterfallFrame->setFrequency(frequency);
    widget.waterfallFrame->setSubRxFrequency(subRxFrequency);
    widget.waterfallFrame->setHigh(band.getWaterfallHigh());
    widget.waterfallFrame->setLow(band.getWaterfallLow());


    widget.spectrumFrame->setBand(band.getStringBand());
    BandLimit limits=band.getBandLimits(band.getFrequency()-(samplerate/2),band.getFrequency()+(samplerate/2));
    widget.spectrumFrame->setBandLimits(limits.min(),limits.max());

}

void UI::modeChanged(int previousMode,int newMode) {

    QString command;

    qDebug() << "UI::modeChanged: " << previousMode << "," << newMode;
    // uncheck previous mode
    switch(previousMode) {
        case MODE_CWL:
            widget.actionCWL->setChecked(FALSE);
            break;
        case MODE_CWU:
            widget.actionCWU->setChecked(FALSE);
            break;
        case MODE_LSB:
            widget.actionLSB->setChecked(FALSE);
            break;
        case MODE_USB:
            widget.actionUSB->setChecked(FALSE);
            break;
        case MODE_DSB:
            widget.actionDSB->setChecked(FALSE);
            break;
        case MODE_AM:
            widget.actionAM->setChecked(FALSE);
            break;
        case MODE_SAM:
            widget.actionSAM->setChecked(FALSE);
            break;
        case MODE_FMN:
            widget.actionFMN->setChecked(FALSE);
            break;
        case MODE_DIGL:
            widget.actionDIGL->setChecked(FALSE);
            break;
        case MODE_DIGU:
            widget.actionDIGU->setChecked(FALSE);
            break;
    }

    // check the new mode and set the filters
    switch(newMode) {
        case MODE_CWL:
            widget.actionCWL->setChecked(TRUE);
            filters.selectFilters(&cwlFilters);
            break;
        case MODE_CWU:
            widget.actionCWU->setChecked(TRUE);
            filters.selectFilters(&cwuFilters);
            break;
        case MODE_LSB:
            widget.actionLSB->setChecked(TRUE);
            filters.selectFilters(&lsbFilters);
            break;
        case MODE_USB:
            widget.actionUSB->setChecked(TRUE);
            filters.selectFilters(&usbFilters);
            break;
        case MODE_DSB:
            widget.actionDSB->setChecked(TRUE);
            filters.selectFilters(&dsbFilters);
            break;
        case MODE_AM:
            widget.actionAM->setChecked(TRUE);
            filters.selectFilters(&amFilters);
            break;
        case MODE_SAM:
            widget.actionSAM->setChecked(TRUE);
            filters.selectFilters(&samFilters);
            break;
        case MODE_FMN:
            widget.actionFMN->setChecked(TRUE);
            filters.selectFilters(&fmnFilters);
            break;
        case MODE_DIGL:
            widget.actionDIGL->setChecked(TRUE);
            filters.selectFilters(&diglFilters);
            break;
        case MODE_DIGU:
            widget.actionDIGU->setChecked(TRUE);
            filters.selectFilters(&diguFilters);
            break;
    }

    widget.spectrumFrame->setMode(mode.getStringMode());
    command.clear(); QTextStream(&command) << "setMode " << mode.getMode();
    connection.sendCommand(command);
  
}

void UI::filtersChanged(FiltersBase* previousFilters,FiltersBase* newFilters) {

    qDebug() << "UI::filtersChanged to " << newFilters->getText();
    // uncheck old filter
    if(previousFilters!=NULL) {
        switch (previousFilters->getSelected()) {
            case 0:
                widget.actionFilter_0->setChecked(FALSE);
                break;
            case 1:
                widget.actionFilter_1->setChecked(FALSE);
                break;
            case 2:
                widget.actionFilter_2->setChecked(FALSE);
                break;
            case 3:
                widget.actionFilter_3->setChecked(FALSE);
                break;
            case 4:
                widget.actionFilter_4->setChecked(FALSE);
                break;
            case 5:
                widget.actionFilter_5->setChecked(FALSE);
                break;
            case 6:
                widget.actionFilter_6->setChecked(FALSE);
                break;
            case 7:
                widget.actionFilter_7->setChecked(FALSE);
                break;
            case 8:
                widget.actionFilter_8->setChecked(FALSE);
                break;
            case 9:
                widget.actionFilter_9->setChecked(FALSE);
                break;
        }
    }

    // set the filter menu text
    widget.actionFilter_0->setText(newFilters->getText(0));
    widget.actionFilter_1->setText(newFilters->getText(1));
    widget.actionFilter_2->setText(newFilters->getText(2));
    widget.actionFilter_3->setText(newFilters->getText(3));
    widget.actionFilter_4->setText(newFilters->getText(4));
    widget.actionFilter_5->setText(newFilters->getText(5));
    widget.actionFilter_6->setText(newFilters->getText(6));
    widget.actionFilter_7->setText(newFilters->getText(7));
    widget.actionFilter_8->setText(newFilters->getText(8));
    widget.actionFilter_9->setText(newFilters->getText(9));

    // check new filter
    if(newFilters!=NULL) {
        switch (newFilters->getSelected()) {
            case 0:
                widget.actionFilter_0->setChecked(TRUE);
                break;
            case 1:
                widget.actionFilter_1->setChecked(TRUE);
                break;
            case 2:
                widget.actionFilter_2->setChecked(TRUE);
                break;
            case 3:
                widget.actionFilter_3->setChecked(TRUE);
                break;
            case 4:
                widget.actionFilter_4->setChecked(TRUE);
                break;
            case 5:
                widget.actionFilter_5->setChecked(TRUE);
                break;
            case 6:
                widget.actionFilter_6->setChecked(TRUE);
                break;
            case 7:
                widget.actionFilter_7->setChecked(TRUE);
                break;
            case 8:
                widget.actionFilter_8->setChecked(TRUE);
                break;
            case 9:
                widget.actionFilter_9->setChecked(TRUE);
                break;
        }
    }

    filters.selectFilter(filters.getFilter());
    widget.spectrumFrame->setFilter(filters.getText());

}

void UI::actionCWL() {
    mode.setMode(MODE_CWL);
    filters.selectFilters(&cwlFilters);
    band.setMode(MODE_CWL);
}

void UI::actionCWU() {
    mode.setMode(MODE_CWU);
    filters.selectFilters(&cwuFilters);
    band.setMode(MODE_CWU);
}

void UI::actionLSB() {
    mode.setMode(MODE_LSB);
    filters.selectFilters(&lsbFilters);
    band.setMode(MODE_LSB);
}

void UI::actionUSB() {
    mode.setMode(MODE_USB);
    filters.selectFilters(&usbFilters);
    band.setMode(MODE_USB);
}

void UI::actionDSB() {
    mode.setMode(MODE_DSB);
    filters.selectFilters(&dsbFilters);
    band.setMode(MODE_DSB);
}

void UI::actionAM() {
    mode.setMode(MODE_AM);
    filters.selectFilters(&amFilters);
    band.setMode(MODE_AM);
}

void UI::actionSAM() {
    mode.setMode(MODE_SAM);
    filters.selectFilters(&samFilters);
    band.setMode(MODE_SAM);
}

void UI::actionFMN() {
    mode.setMode(MODE_FMN);
    filters.selectFilters(&fmnFilters);
    band.setMode(MODE_FMN);
}

void UI::actionDIGL() {
    mode.setMode(MODE_DIGL);
    filters.selectFilters(&diglFilters);
    band.setMode(MODE_DIGL);
}

void UI::actionDIGU() {
    mode.setMode(MODE_DIGU);
    filters.selectFilters(&diguFilters);
    band.setMode(MODE_DIGU);
}

void UI::actionFilter0() {
    filters.selectFilter(0);
}

void UI::actionFilter1() {
    filters.selectFilter(1);
}

void UI::actionFilter2() {
    filters.selectFilter(2);
}

void UI::actionFilter3() {
    filters.selectFilter(3);
}

void UI::actionFilter4() {
    filters.selectFilter(4);
}

void UI::actionFilter5() {
    filters.selectFilter(5);
}

void UI::actionFilter6() {
    filters.selectFilter(6);
}

void UI::actionFilter7() {
    filters.selectFilter(7);
}

void UI::actionFilter8() {
    filters.selectFilter(8);
}

void UI::actionFilter9() {
    filters.selectFilter(9);
}

void UI::filterChanged(int previousFilter,int newFilter) {
    QString command;

    qDebug() << "UI::filterChanged: " << previousFilter << ":" << newFilter;

    int low,high;
    switch(previousFilter) {
        case 0:
            widget.actionFilter_0->setChecked(FALSE);
            break;
        case 1:
            widget.actionFilter_1->setChecked(FALSE);
            break;
        case 2:
            widget.actionFilter_2->setChecked(FALSE);
            break;
        case 3:
            widget.actionFilter_3->setChecked(FALSE);
            break;
        case 4:
            widget.actionFilter_4->setChecked(FALSE);
            break;
        case 5:
            widget.actionFilter_5->setChecked(FALSE);
            break;
        case 6:
            widget.actionFilter_6->setChecked(FALSE);
            break;
        case 7:
            widget.actionFilter_7->setChecked(FALSE);
            break;
        case 8:
            widget.actionFilter_8->setChecked(FALSE);
            break;
        case 9:
            widget.actionFilter_9->setChecked(FALSE);
            break;
    }

    switch(newFilter) {
        case 0:
            widget.actionFilter_0->setChecked(TRUE);
            break;
        case 1:
            widget.actionFilter_1->setChecked(TRUE);
            break;
        case 2:
            widget.actionFilter_2->setChecked(TRUE);
            break;
        case 3:
            widget.actionFilter_3->setChecked(TRUE);
            break;
        case 4:
            widget.actionFilter_4->setChecked(TRUE);
            break;
        case 5:
            widget.actionFilter_5->setChecked(TRUE);
            break;
        case 6:
            widget.actionFilter_6->setChecked(TRUE);
            break;
        case 7:
            widget.actionFilter_7->setChecked(TRUE);
            break;
        case 8:
            widget.actionFilter_8->setChecked(TRUE);
            break;
        case 9:
            widget.actionFilter_9->setChecked(TRUE);
            break;
    }

    if(mode.getMode()==MODE_CWL) {
        low=-cwPitch-filters.getLow();
        high=-cwPitch+filters.getHigh();
    } else if(mode.getMode()==MODE_CWU) {
        low=cwPitch-filters.getLow();
        high=cwPitch+filters.getHigh();
    } else {
        low=filters.getLow();
        high=filters.getHigh();
    }

    command.clear(); QTextStream(&command) << "setFilter " << low << " " << high;
    connection.sendCommand(command);
    widget.spectrumFrame->setFilter(low,high);
    widget.spectrumFrame->setFilter(filters.getText());
    widget.waterfallFrame->setFilter(low,high);
    band.setFilter(newFilter);
}

void UI::frequencyChanged(long long f) {
    QString command;
    //qDebug() << __FUNCTION__ << ": " << f;

    frequency=f;
    command.clear();
    QTextStream(&command) << "setFrequency " << f;

    band.setFrequency(f);
    connection.sendCommand(command);
    widget.spectrumFrame->setFrequency(f);
    widget.waterfallFrame->setFrequency(f);
}

void UI::frequencyMoved(int increment,int step) {
    QString command;

    //qDebug() << __FUNCTION__ << ": increment=" << increment << " step=" << step;

    long long f;

    if(subRx) {
        long long diff;
        long long frequency = band.getFrequency();
        f=subRxFrequency+(long long)(increment*step);
        int samplerate = widget.spectrumFrame->samplerate();
        if ((f >= (frequency - (samplerate / 2))) && (f <= (frequency + (samplerate / 2)))) {
            subRxFrequency = f;
        }
        diff = frequency - subRxFrequency;
        command.clear(); QTextStream(&command) << "SetSubRXFrequency " << diff;
        connection.sendCommand(command);
        widget.spectrumFrame->setSubRxFrequency(subRxFrequency);
        widget.waterfallFrame->setSubRxFrequency(subRxFrequency);
        setSubRxPan();

    } else {
        band.setFrequency(band.getFrequency()-(long long)(increment*step));
        frequency=band.getFrequency();
        command.clear(); QTextStream(&command) << "setFrequency " << frequency;
        widget.spectrumFrame->setFrequency(frequency);
        widget.waterfallFrame->setFrequency(frequency);
        connection.sendCommand(command);
    }
}

void UI::actionANF() {
    QString command;
    command.clear(); QTextStream(&command) << "SetANF " << (widget.actionANF->isChecked()?"true":"false");
    connection.sendCommand(command);
}

void UI::actionNR() {
    QString command;
    command.clear(); QTextStream(&command) << "SetNR " << (widget.actionNR->isChecked()?"true":"false");
    connection.sendCommand(command);
}

void UI::actionNB() {
    QString command;
    command.clear(); QTextStream(&command) << "SetNB " << (widget.actionNB->isChecked()?"true":"false");
    connection.sendCommand(command);
}

void UI::actionSDROM() {
    QString command;
    command.clear(); QTextStream(&command) << "SetSDROM " << (widget.actionSDROM->isChecked()?"true":"false");
    connection.sendCommand(command);
}

void UI::actionPolyphase() {
    QString command;
    command.clear(); QTextStream(&command) << "SetSpectrumPolyphase " << (widget.actionPolyphase->isChecked()?"true":"false");
    connection.sendCommand(command);
}

void UI::actionSlow() {
    QString command;
    // reset the current selection
    switch(agc) {
    case AGC_LONG:
        widget.actionLong->setChecked(FALSE);
        break;
    case AGC_SLOW:
        widget.actionSlow->setChecked(FALSE);
        break;
    case AGC_MEDIUM:
        widget.actionMedium->setChecked(FALSE);
        break;
    case AGC_FAST:
        widget.actionFast->setChecked(FALSE);
        break;
    }
    agc=AGC_SLOW;

    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);

}

void UI::actionMedium() {
    QString command;

    // reset the current selection
    switch(agc) {
    case AGC_LONG:
        widget.actionLong->setChecked(FALSE);
        break;
    case AGC_SLOW:
        widget.actionSlow->setChecked(FALSE);
        break;
    case AGC_MEDIUM:
        widget.actionMedium->setChecked(FALSE);
        break;
    case AGC_FAST:
        widget.actionFast->setChecked(FALSE);
        break;
    }
    agc=AGC_MEDIUM;

    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);

}

void UI::actionFast() {
    QString command;
    // reset the current selection
    switch(agc) {
    case AGC_LONG:
        widget.actionLong->setChecked(FALSE);
        break;
    case AGC_SLOW:
        widget.actionSlow->setChecked(FALSE);
        break;
    case AGC_MEDIUM:
        widget.actionMedium->setChecked(FALSE);
        break;
    case AGC_FAST:
        widget.actionFast->setChecked(FALSE);
        break;
    }
    agc=AGC_FAST;

    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);
}

void UI::actionLong() {
    QString command;
    // reset the current selection
    switch(agc) {
    case AGC_LONG:
        widget.actionLong->setChecked(FALSE);
        break;
    case AGC_SLOW:
        widget.actionSlow->setChecked(FALSE);
        break;
    case AGC_MEDIUM:
        widget.actionMedium->setChecked(FALSE);
        break;
    case AGC_FAST:
        widget.actionFast->setChecked(FALSE);
        break;
    }
    agc=AGC_LONG;
    command.clear(); QTextStream(&command) << "SetAGC " << agc;
    connection.sendCommand(command);
}

void UI::setSubRxPan() {
    QString command;
    float pan;

    // set the pan relative to frequency position (only works when in stereo)
    pan=(float)(subRxFrequency-(frequency-(sampleRate/2)))/(float)sampleRate;

    if(pan<0.0) pan=0.0;
    if(pan>1.0) pan=1.0;
    command.clear(); QTextStream(&command) << "SetSubRXPan " << pan;
    connection.sendCommand(command);
}

void UI::actionMuteMainRx() {
    QString command;
    int g=gain;

    if(widget.actionMuteMainRx->isChecked()) {
        g=0;
    }

    command.clear(); QTextStream(&command) << "SetRXOutputGain " << g;
    connection.sendCommand(command);
}

void UI::actionMuteSubRx() {
    QString command;
    int g=subRxGain;

    if(widget.actionMuteSubRx->isChecked()) {
        g=0;
    }

    command.clear(); QTextStream(&command) << "SetSubRXOutputGain " << g;
    connection.sendCommand(command);

}

void UI::actionPreamp() {
    QString command;
    command.clear(); QTextStream(&command) << "SetPreamp " << (widget.actionPreamp->isChecked()?"on":"off");
    connection.sendCommand(command);
}

void UI::actionBandscope() {
    if(widget.actionBandscope->isChecked()) {
        if(bandscope==NULL) bandscope=new Bandscope();
        bandscope->setWindowTitle("QtRadio Bandscope");
        bandscope->show();
        bandscope->connect(configure.getHost());
    } else {
        if(bandscope!=NULL) {
            bandscope->setVisible(FALSE);
            bandscope->disconnect();
        }
    }
}

void UI::actionRecord() {
    QString command;
    command.clear(); QTextStream(&command) << "record " << (widget.actionRecord->isChecked()?"on":"off");
    connection.sendCommand(command);
}

void UI::actionGain_10() {
    actionGain(10);
}

void UI::actionGain_20() {
    actionGain(20);
}

void UI::actionGain_30() {
    actionGain(30);
}

void UI::actionGain_40() {
    actionGain(40);
}

void UI::actionGain_50() {
    actionGain(50);
}

void UI::actionGain_60() {
    actionGain(60);
}

void UI::actionGain_70() {
    actionGain(70);
}

void UI::actionGain_80() {
    actionGain(80);
}

void UI::actionGain_90() {
    actionGain(90);
}

void UI::actionGain_100() {
    actionGain(100);
}

void UI::actionGain(int g) {
    QString command;
    setGain(false);
    gain=g;
    subRxGain=g;
    setGain(true);
    command.clear(); QTextStream(&command) << "SetRXOutputGain " << g;
    connection.sendCommand(command);
    command.clear(); QTextStream(&command) << "SetSubRXOutputGain " << g;
    connection.sendCommand(command);
}

void UI::setGain(bool state) {
    switch(gain) {
    case 10:
        widget.actionGain_10->setChecked(state);
        break;
    case 20:
        widget.actionGain_20->setChecked(state);
        break;
    case 30:
        widget.actionGain_30->setChecked(state);
        break;
    case 40:
        widget.actionGain_40->setChecked(state);
        break;
    case 50:
        widget.actionGain_50->setChecked(state);
        break;
    case 60:
        widget.actionGain_60->setChecked(state);
        break;
    case 70:
        widget.actionGain_70->setChecked(state);
        break;
    case 80:
        widget.actionGain_80->setChecked(state);
        break;
    case 90:
        widget.actionGain_90->setChecked(state);
        break;
    case 100:
        widget.actionGain_100->setChecked(state);
        break;
    }
}

void UI::nrValuesChanged(int taps,int delay,double gain,double leakage) {
    QString command;
    command.clear(); QTextStream(&command) << "SetNRVals " << taps << " " << delay << " "
                                           << gain << " " << leakage;
    connection.sendCommand(command);


}

void UI::anfValuesChanged(int taps,int delay,double gain,double leakage) {
    QString command;
    command.clear(); QTextStream(&command) << "SetANFVals " << taps<< " " << delay << " "
                                           << gain << " " << leakage;
    connection.sendCommand(command);
}

void UI::nbThresholdChanged(double threshold) {
    QString command;
    command.clear(); QTextStream(&command) << "SetNBVals " << threshold;
    connection.sendCommand(command);
}

void UI::sdromThresholdChanged(double threshold) {
    QString command;
    command.clear(); QTextStream(&command) << "SetSDROMVals " << threshold;
    connection.sendCommand(command);
}

void UI::actionBookmark() {
    QString strFrequency=stringFrequency(frequency);
    bookmarkDialog.setTitle(strFrequency);
    bookmarkDialog.setBand(band.getStringBand());
    bookmarkDialog.setFrequency(strFrequency);
    bookmarkDialog.setMode(mode.getStringMode());
    bookmarkDialog.setFilter(filters.getText());
    bookmarkDialog.show();
}

void UI::addBookmark() {
    qDebug() << "addBookmark";
    Bookmark* bookmark=new Bookmark();
    bookmark->setTitle(bookmarkDialog.getTitle());
    bookmark->setBand(band.getBand());
    bookmark->setFrequency(band.getFrequency());
    bookmark->setMode(mode.getMode());
    bookmark->setFilter(filters.getFilter());
    bookmarks.add(bookmark);
    bookmarks.buildMenu(widget.menuView_Bookmarks);
}

void UI::selectBookmark(QAction* action) {
    QString command;

    bookmarks.select(action);

    band.selectBand(bookmarks.getBand());

    frequency=bookmarks.getFrequency();
    band.setFrequency(frequency);
    command.clear(); QTextStream(&command) << "setFrequency " << frequency;
    connection.sendCommand(command);

    widget.spectrumFrame->setFrequency(frequency);
    widget.waterfallFrame->setFrequency(frequency);

    mode.setMode(bookmarks.getMode());

    filters.selectFilter(bookmarks.getFilter());

}

void UI::selectABookmark() {
/*
    int entry=bookmarksDialog->getSelected();
    if(entry>=0 && entry<bookmarks.count()) {
        selectBookmark(entry);
    }
*/
}

void UI::editBookmarks() {
    bookmarksEditDialog=new BookmarksEditDialog(this,&bookmarks);
    bookmarksEditDialog->setVisible(true);
    connect(bookmarksEditDialog,SIGNAL(bookmarkDeleted(int)),this,SLOT(bookmarkDeleted(int)));
    connect(bookmarksEditDialog,SIGNAL(bookmarkUpdated(int,QString)),this,SLOT(bookmarkUpdated(int,QString)));
    connect(bookmarksEditDialog,SIGNAL(bookmarkSelected(int)),this,SLOT(bookmarkSelected(int)));
}

void UI::bookmarkDeleted(int entry) {
    //qDebug() << "UI::bookmarkDeleted: " << entry;
    bookmarks.remove(entry);
    bookmarks.buildMenu(widget.menuView_Bookmarks);
}

void UI::bookmarkUpdated(int entry,QString title) {
    if(entry>=0 && entry<bookmarks.count()) {
        Bookmark* bookmark=bookmarks.at(entry);
        bookmark->setTitle(title);
    }
}

void UI::bookmarkSelected(int entry) {

    //qDebug() << "UI::bookmarkSelected " << entry;
    if(entry>=0 && entry<bookmarks.count()) {
        Bookmark* bookmark=bookmarks.at(entry);
        FiltersBase* filters;

        bookmarksEditDialog->setTitle(bookmark->getTitle());
        bookmarksEditDialog->setBand(band.getStringBand(bookmark->getBand()));
        bookmarksEditDialog->setFrequency(stringFrequency(bookmark->getFrequency()));
        bookmarksEditDialog->setMode(mode.getStringMode(bookmark->getMode()));

        switch(bookmark->getMode()) {
        case MODE_CWL:
            filters=&cwlFilters;
            break;
        case MODE_CWU:
            filters=&cwuFilters;
            break;
        case MODE_LSB:
            filters=&lsbFilters;
            break;
        case MODE_USB:
            filters=&usbFilters;
            break;
        case MODE_DSB:
            filters=&dsbFilters;
            break;
        case MODE_AM:
            filters=&amFilters;
            break;
        case MODE_SAM:
            filters=&samFilters;
            break;
        case MODE_FMN:
            filters=&fmnFilters;
            break;
        case MODE_DIGL:
            filters=&diglFilters;
            break;
        case MODE_DIGU:
            filters=&diguFilters;
            break;
        }
        bookmarksEditDialog->setFilter(filters->getText(bookmark->getFilter()));
    } else {
        bookmarksEditDialog->setTitle("");
        bookmarksEditDialog->setBand("");
        bookmarksEditDialog->setFrequency("");
        bookmarksEditDialog->setMode("");
        bookmarksEditDialog->setFilter("");
    }
}

QString UI::stringFrequency(long long frequency) {
    QString strFrequency;
    strFrequency.sprintf("%lld.%03lld.%03lld",frequency/1000000,frequency%1000000/1000,frequency%1000);
    return strFrequency;
}

void UI::addXVTR(QString title,long long minFrequency,long long maxFrequency,long long ifFrequency,long long freq,int m,int filt) {

    qDebug()<<"UI::addXVTR"<<title;
    xvtr.add(title,minFrequency,maxFrequency,ifFrequency,freq,m,filt);

    // update the menu
    xvtr.buildMenu(widget.menuXVTR);
    configure.updateXvtrList(&xvtr);
}

void UI::deleteXVTR(int index) {
    xvtr.del(index);

    // update the menu
    xvtr.buildMenu(widget.menuXVTR);
    configure.updateXvtrList(&xvtr);
}

void UI::selectXVTR(QAction* action) {
    xvtr.select(action);
}
