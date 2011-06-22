/* 
 * File:   qtMonitor.h
 * Author: john
 *
 * Created on 05 August 2010, 12:20
 */

#ifndef _QTMONITOR_H
#define	_QTMONITOR_H

#include <QTcpSocket>
#include <QTimer>
#include <QSemaphore>

#include <audio.h>
#include "ui_qtMonitor.h"

class qtMonitor : public QMainWindow {
    Q_OBJECT
public:
    qtMonitor();
    virtual ~qtMonitor();
    void sendCommand(QString command);
public slots:
    void band_160_buttonPressed();
    void band_80_buttonPressed();
    void band_60_buttonPressed();
    void band_40_buttonPressed();
    void band_30_buttonPressed();
    void band_20_buttonPressed();
    void band_17_buttonPressed();
    void band_15_buttonPressed();
    void band_12_buttonPressed();
    void band_10_buttonPressed();
    void band_6_buttonPressed();
    void band_gen_buttonPressed();

    void mode_lsb_buttonPressed();
    void mode_usb_buttonPressed();
    void mode_dsb_buttonPressed();
    void mode_cwl_buttonPressed();
    void mode_cwu_buttonPressed();
    void mode_am_buttonPressed();

    void connect_buttonPressed();

    void socketError(QAbstractSocket::SocketError socketError);
    void connected();
    void socketData();

    void update();

    void setMode(int mode);
    void setFilter(int low,int high);
    void setFrequency(long long frequency);
    void moveFrequency(int f);
    void setGain(int gain);

    void audioChanged(int choice);

private:
    Ui::qtMonitor widget;

    QTcpSocket* tcpSocket;

    QSemaphore sem;
    audio audio_device;

    int fps;

    int mode;

    int filterLow;
    int filterHigh;

    long long frequency;

};

#endif	/* _QTMONITOR_H */
