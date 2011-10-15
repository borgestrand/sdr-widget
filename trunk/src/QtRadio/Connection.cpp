/* 
 * File:   Connection.cpp
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

#include "Connection.h"

Connection::Connection() {
    //qDebug() << "Connection::Connection";
    tcpSocket=NULL;
    state=READ_HEADER_TYPE;
    bytes=0;
    hdr=(char*)malloc(HEADER_SIZE);  // HEADER_SIZE is larger than AUTIO_HEADER_SIZE so it is OK
                                    // for both
    SemSpectrum.release();
}

Connection::Connection(const Connection& orig) {
}

Connection::~Connection() {
}

void Connection::connect(QString h,int p) {
    host=h;
    port=p;
    tcpSocket=new QTcpSocket(this);

    QObject::connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)));

    QObject::connect(tcpSocket, SIGNAL(connected()),
            this, SLOT(connected()));

    QObject::connect(tcpSocket, SIGNAL(disconnected()),
            this, SLOT(disconnected()));

    QObject::connect(tcpSocket, SIGNAL(readyRead()),
            this, SLOT(socketData()));

    qDebug() << "Connection::connect: connectToHost: " << host << ":" << port;
    tcpSocket->connectToHost(host,port);


}

void Connection::disconnected() {
    emit disconnected("Remote disconnected");
}

void Connection::disconnect() {

    //qDebug() << "Connection::disconnect";
    if(tcpSocket!=NULL) {
        tcpSocket->close();
        tcpSocket=NULL;
    }
}

void Connection::socketError(QAbstractSocket::SocketError socketError) {
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            qDebug() << "Remote closed connection";
            break;
        case QAbstractSocket::HostNotFoundError:
            qDebug() << "Host not found";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            qDebug() << "Remote host refused connection";
            break;
        default:
            qDebug() << "Socket Error: " << tcpSocket->errorString();
    }

    emit disconnected(tcpSocket->errorString());
    tcpSocket=NULL;
}

void Connection::connected() {
    //qDebug() << "Connected" << tcpSocket->isValid();
    emit isConnected();
}

void Connection::sendCommand(QString command) {

    if(tcpSocket!=NULL) {
        mutex.lock();
        char buffer[64];
        if(command.length()>=64) qDebug() << "command too long: " << command;
        //qDebug() << "sendCommand:" << command;
        strcpy(buffer,command.toUtf8().constData());
        tcpSocket->write(buffer,64);
        tcpSocket->flush();
        mutex.unlock();
    }
}

void Connection::sendAudio(int length, char* data) {
    QString command;
    char buffer[64];

    QTextStream(&command) << "mic ";
    strcpy(buffer,command.toUtf8().constData());
    if (length >= 59){
        qDebug() << "mic audio data exceeds buffer: " << length;
        length = 59;
    }
    memcpy(&buffer[4], data, length);

    if(tcpSocket!=NULL) {
        mutex.lock();
        tcpSocket->write(buffer,64);
        tcpSocket->flush();
        mutex.unlock();
    }
}

void Connection::socketData() {

    int toRead;
    int bytesRead=0;
    int thisRead;

    toRead=tcpSocket->bytesAvailable();
    while(bytesRead<toRead) {
        switch(state) {
        case READ_HEADER_TYPE:
            thisRead=tcpSocket->read(&hdr[0],1);
            if (thisRead == 1) bytes++;
            if (hdr[0] == AUDIO_BUFFER) state=READ_AUDIO_HEADER;
            else state=READ_HEADER;
            break;

        case READ_AUDIO_HEADER:
            thisRead=tcpSocket->read(&hdr[bytes],AUDIO_HEADER_SIZE - bytes);
            bytes+=thisRead;
            if ((bytes == AUDIO_HEADER_SIZE)){
                    length = atoi(&hdr[AUDIO_LENGTH_POSITION]);
                    if ((length < 0) || (length > 4800 * 8)){
                        state = READ_HEADER_TYPE;
                    }
                    else {
                        buffer = (char*)malloc(length);
                        bytes = 0;
                        state = READ_BUFFER;
                    }
             }
            break;

         case READ_HEADER:
            thisRead=tcpSocket->read(&hdr[bytes],HEADER_SIZE - bytes);
            bytes+=thisRead;
            if(bytes==HEADER_SIZE) {
                length=atoi(&hdr[26]);
                if ((length < 0) || (length > 4096)){
                        state = READ_HEADER_TYPE;
                }
                else {
                    buffer=(char*)malloc(length);
                    bytes=0;
                    state=READ_BUFFER;
                }
            }
            break;

        case READ_BUFFER:
            thisRead=tcpSocket->read(&buffer[bytes],length-bytes);
            bytes+=thisRead;
            //qDebug() << "READ_BUFFER: read " << bytes << " of " << length;
            if(bytes==length) {
                queue.enqueue(new Buffer(hdr,buffer));
                QTimer::singleShot(0,this,SLOT(processBuffer()));
                hdr=(char*)malloc(HEADER_SIZE);
                bytes=0;
                state=READ_HEADER_TYPE;
            }
            break;
        }
        bytesRead+=thisRead;
    }
}


void Connection::processBuffer() {
    Buffer* buffer;
    char* nextHeader;
    char* nextBuffer;

    while (!queue.isEmpty()){
        buffer=queue.dequeue();
        nextHeader=buffer->getHeader();
        nextBuffer=buffer->getBuffer();
        // emit a signal to show what buffer we have
        //qDebug() << "processBuffer " << nextHeader[0];
        if(nextHeader[0]==SPECTRUM_BUFFER){
            emit spectrumBuffer(nextHeader,nextBuffer);
        }
        else if(nextHeader[0]==AUDIO_BUFFER) {
            emit audioBuffer(nextHeader,nextBuffer);
        } else if(nextHeader[0]==BANDSCOPE_BUFFER) {
            //qDebug() << "socketData: bandscope";
            emit bandscopeBuffer(nextHeader,nextBuffer);
        } else {
            qDebug() << "Connection::socketData: invalid header: " << nextHeader[0];
            queue.clear();
        }
    }
}

void Connection::freeBuffers(char* header,char* buffer) {
    if (header != NULL) free(header);
    if (buffer != NULL) free(buffer);
}
