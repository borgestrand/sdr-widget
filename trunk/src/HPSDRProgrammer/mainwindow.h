/*
 * File:   mainwindow.h
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 23 November 2010
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <pcap.h>

#include <QMainWindow>
#include <QUdpSocket>
#include <QHostAddress>

#include "Interfaces.h"
#include "ReceiveThread.h"
#include "RawReceiveThread.h"
#include "Discovery.h"
#include "Metis.h"
#include "AboutDialog.h"
#include "Version.h"

// states
#define IDLE 0
#define ERASING 1
#define PROGRAMMING 2
#define ERASING_ONLY 3
#define READ_MAC 4
#define READ_IP 5
#define WRITE_IP 6
#define JTAG_INTERROGATE 7
#define JTAG_PROGRAM 8
#define FLASH_ERASING 9
#define FLASH_PROGRAM 10


#define TIMEOUT 10 // ms
#define MAX_ERASE_TIMEOUTS (2000) // 20 seconds

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
#ifdef Q_WS_MAC
    void setPath(char* path);
#endif


public slots:
    void quit();
    void about();
    void interfaceSelected(int);
    void browse();
    void program();
    void erase();
    void getMAC();
    void getIP();
    void setIP();

    void discover();
    void discovery_timeout();

    // SLOTS for RawReceiveThread
    void erase_timeout();
    void eraseCompleted();
    void nextBuffer();
    void timeout();
    void macAddress(unsigned char*);
    void ipAddress(unsigned char*);
    void fpgaId(unsigned char*);

    void metis_found(Metis*);
    void metisSelected(int);

    void tabChanged(int);

    void jtagInterrogate();
    void jtagBrowse();
    void jtagProgram();
    void jtagFlashBrowse();
    void jtagFlashProgram();
    void nextJTAGBuffer();
    void startJTAGFlashErase();

private:
    Ui::MainWindow *ui;

    //void loadPOF(QString filename);
    int loadRBF(QString filename);
    void eraseData();
    void readMAC();
    void readIP();
    void writeIP();

    void sendRawCommand(unsigned char command);
    void sendCommand(unsigned char command);

    void sendRawData();
    void sendData();
    void sendJTAGData();
    void sendJTAGFlashData();

    void idle();
    void status(QString text);

    void bootloaderProgram();
    void flashProgram();
    void bootloaderErase();
    void flashErase();

    int loadMercuryRBF(QString filename);
    int loadPenelopeRBF(QString filename);
    void jtagBootloaderProgram();
    void jtagEraseData();
    //void jtagFlashProgram();
    void loadFlash();

#ifdef Q_WS_MAC
    char* myPath;
#endif

    Interfaces interfaces;
    long ip;
    QString interfaceName;
    QString hwAddress;
    unsigned char hw[6];

    long metisIP;
    QString metisHostAddress;

    int s;

    char* data;
    int offset;
    int start;
    int end;
    int blocks;

    int size; //depending on how and what we are programming
              // 0 if programming flash on metis in raw modfe
              // blocks if programming flash on metis in command mode
              // bytes (blocks*256) if programming flash on JTAG (Mercury or Penelope)
    unsigned char data_command;

    pcap_t *handle;

    QList<Metis*> metis;

    int state;

    int percent;
    int eraseTimeouts;

    Discovery* discovery;
    ReceiveThread* receiveThread;
    RawReceiveThread* rawReceiveThread;

    bool bootloader;

    long fpga_id;

    AboutDialog aboutDialog;

};

#endif // MAINWINDOW_H
