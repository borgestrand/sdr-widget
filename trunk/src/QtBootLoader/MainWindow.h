/*
 * File:   MainWindow.h
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
#include "Interfaces.h"
#include "RawReceiveThread.h"

// states
#define IDLE 0
#define ERASING 1
#define PROGRAMMING 2
#define ERASING_ONLY 3
#define READ_MAC 4
#define READ_IP 5
#define WRITE_IP 6

#define TIMEOUT 10 // ms
#define MAX_ERASE_TIMEOUTS (20*(1000/TIMEOUT)) // 20 seconds

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void interfaceSelected(int);
    void browse();
    void program();
    void erase();
    void getMAC();
    void getIP();
    void setIP();

    // SLOTS for RawReceiveThread
    void commandCompleted();
    void nextBuffer();
    void timeout();
    void macAddress(unsigned char*);
    void ipAddress(unsigned char*);

private:
    Ui::MainWindow *ui;

    void eraseData();
    void readMAC();
    void readIP();
    void writeIP();
    void sendCommand(unsigned char command);
    void sendData();

    void idle();
    void status(QString text);

    Interfaces interfaces;
    long ip;
    QString hwAddress;
    unsigned char hw[6];

    char* data;
    int offset;
    int start;
    int end;

    pcap_t *handle;

    int state;

    int percent;
    int eraseTimeouts;

    RawReceiveThread* rawReceiveThread;
};

#endif // MAINWINDOW_H
