/*
 * File:   RawReceiveThread.h
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

#ifndef RAWRECEIVETHREAD_H
#define RAWRECEIVETHREAD_H

#include <pcap.h>

#include <QThread>


class RawReceiveThread : public QThread {
    Q_OBJECT
public:
    RawReceiveThread(unsigned char*,pcap_t*);
    void run();
    void stop();
signals:
    void commandCompleted();
    void nextBuffer();
    void macAddress(unsigned char*);
    void ipAddress(unsigned char*);
    void timeout();
private:
    bool stopped;
    unsigned char* hw;
    pcap_t* handle;
};

#endif // RAWRECEIVETHREAD_H
