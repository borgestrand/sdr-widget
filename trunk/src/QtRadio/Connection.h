/* 
 * File:   Connection.h
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 16 August 2010, 07:40
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

#ifndef CONNECTION_H
#define	CONNECTION_H

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QTimer>
#include <QMutex>
#include <QQueue>

#include "Buffer.h"

#define SPECTRUM_BUFFER     0
#define AUDIO_BUFFER        1
#define BANDSCOPE_BUFFER    2

#define HEADER_SIZE 48

#define READ_HEADER 0
#define READ_BUFFER 1

class Connection : public QObject {
    Q_OBJECT
public:
    Connection();
    Connection(const Connection& orig);
    virtual ~Connection();
    void connect(QString host,int receiver);
    void disconnect();
    void sendCommand(QString command);
    void freeBuffers(char* header,char* buffer);

public slots:
    void connected();
    void disconnected();
    void socketError(QAbstractSocket::SocketError socketError);
    void socketData();
    void processBuffer();

signals:
    void isConnected();
    void disconnected(QString message);
    void header(char* header);
    void audioBuffer(char* header,char* buffer);
    void spectrumBuffer(char* header,char* buffer);
    void bandscopeBuffer(char* header,char* buffer);

private:
    QString host;
    int port;
    QTcpSocket* tcpSocket;
    QMutex mutex;
    int state;
    char* hdr;
    char* buffer;
    int length;
    int bytes;

    QQueue<Buffer*> queue;
};

#endif	/* CONNECTION_H */

