/* 
 * File:   UI.h
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

#ifndef _UI_H
#define	_UI_H

#include "ui_UI.h"

#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QtMultimedia/QAudioFormat>
#include <QVector>

#include "About.h"
#include "Configure.h"
#include "Audio.h"
#include "Connection.h"
#include "Spectrum.h"
#include "Band.h"
#include "BandLimit.h"
#include "Mode.h"
#include "Filters.h"
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
#include "Bandscope.h"
#include "BookmarkDialog.h"
#include "Bookmark.h"
#include "Bookmarks.h"
#include "BookmarksDialog.h"
#include "BookmarksEditDialog.h"
#include "Xvtr.h"
#include "XvtrEntry.h"
#include "KeypadDialog.h"

#define DSPSERVER_BASE_PORT 8000

#define AGC_LONG 1
#define AGC_SLOW 2
#define AGC_MEDIUM 3
#define AGC_FAST 4

class UI : public QMainWindow {
    Q_OBJECT
public:
    UI();
    virtual ~UI();
    void loadSettings();
    void saveSettings();
    void closeEvent(QCloseEvent* event);

    void sendCommand(QString command);

public slots:
    void actionConfigure();
    void actionAbout();
    void actionConnect();
    void actionDisconnect();
    void actionSubRx();
    void actionBandscope();
    void actionRecord();

    void actionMuteMainRx();
    void actionMuteSubRx();

    void actionGain_10();
    void actionGain_20();
    void actionGain_30();
    void actionGain_40();
    void actionGain_50();
    void actionGain_60();
    void actionGain_70();
    void actionGain_80();
    void actionGain_90();
    void actionGain_100();

    void actionKeypad();
    void setKeypadFrequency(long long);

    void action160();
    void action80();
    void action60();
    void action40();
    void action30();
    void action20();
    void action17();
    void action15();
    void action12();
    void action10();
    void action6();
    void actionGen();
    void actionWWV();

    void actionCWL();
    void actionCWU();
    void actionLSB();
    void actionUSB();
    void actionDSB();
    void actionAM();
    void actionSAM();
    void actionFMN();
    void actionDIGL();
    void actionDIGU();

    void actionFilter0();
    void actionFilter1();
    void actionFilter2();
    void actionFilter3();
    void actionFilter4();
    void actionFilter5();
    void actionFilter6();
    void actionFilter7();
    void actionFilter8();
    void actionFilter9();

    void actionANF();
    void actionNR();
    void actionNB();
    void actionSDROM();

    void actionPolyphase();

    void actionSlow();
    void actionMedium();
    void actionFast();
    void actionLong();

    void actionPreamp();

    void connected();
    void disconnected(QString message);
    void audioBuffer(char* header,char* buffer);
    void spectrumBuffer(char* header,char* buffer);

    void bandChanged(int previousBand,int newBand);
    void modeChanged(int previousMode,int newMode);
    void filtersChanged(FiltersBase* previousFilters,FiltersBase* newFilters);
    void filterChanged(int previousFilter,int newFilter);
    void frequencyChanged(long long frequency);

    void updateSpectrum();

    void audioDeviceChanged(QAudioDeviceInfo info,int rate,int channels,QAudioFormat::Endian byteOrder);
    //void sampleRateChanged(int choice);
    //void audioChannelsChanged(int channels);
    void encodingChanged(int choice);
    //void byteOrderChanged(int choice);


    void setSubRxGain(int gain);

    void frequencyMoved(int increment,int step);

    void spectrumHighChanged(int high);
    void spectrumLowChanged(int low);
    void fpsChanged(int f);
    void waterfallHighChanged(int high);
    void waterfallLowChanged(int low);
    void waterfallAutomaticChanged(bool state);

    void hostChanged(QString host);
    void receiverChanged(int rx);

    void nrValuesChanged(int,int,double,double);
    void anfValuesChanged(int,int,double,double);
    void nbThresholdChanged(double);
    void sdromThresholdChanged(double);

    void actionBookmark();
    void addBookmark();
    void selectABookmark();
    void editBookmarks();
    void bookmarkDeleted(int);
    void bookmarkUpdated(int,QString);
    void bookmarkSelected(int entry);

    void addXVTR(QString,long long,long long,long long,long long,int,int);
    void deleteXVTR(int index);
    void selectXVTR(QAction* action);
    void selectBookmark(QAction* action);

signals:
    void subRxStateChanged(bool state);

private:
    void setSubRxPan();
    void actionGain(int g);
    void setGain(bool state);


    void appendBookmark(Bookmark* bookmark);

    QString stringFrequency(long long frequency);

    Ui::UI widget;

    Audio audio;
    int audio_device;
    int audio_sample_rate;
    int audio_channels;
    int audio_buffers;
    QAudioFormat::Endian audio_byte_order;
    char* first_audio_buffer;
    char* first_audio_header;
    int gain;
    int subRxGain;
    bool subRx;

    long long subRxFrequency;
    Connection connection;
    Band band;
    Mode mode;
    Filters filters;
    CWLFilters cwlFilters;
    CWUFilters cwuFilters;
    LSBFilters lsbFilters;
    USBFilters usbFilters;
    DSBFilters dsbFilters;
    AMFilters amFilters;
    SAMFilters samFilters;
    FMNFilters fmnFilters;
    DIGUFilters diguFilters;
    DIGLFilters diglFilters;

    Xvtr xvtr;

    int agc;

    int cwPitch;

    long long frequency;

    int fps;

    About about;
    Configure configure;
    int sampleRate;

    Bandscope* bandscope;

    BookmarkDialog bookmarkDialog;
    BookmarksDialog* bookmarksDialog;
    BookmarksEditDialog* bookmarksEditDialog;

    Bookmarks bookmarks;

    KeypadDialog keypad;

};

#endif	/* _UI_H */
