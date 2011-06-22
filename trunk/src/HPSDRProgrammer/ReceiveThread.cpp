#include <errno.h>

#include "QDebug"

#include "ReceiveThread.h"
#include "mainwindow.h"

ReceiveThread::ReceiveThread(QString myip,QString metis) {
    qDebug()<<"ReceiveThread: "<<myip;
    metisip=metis;

    if(!socket.bind(QHostAddress(myip),5001,QUdpSocket::ReuseAddressHint)) {
        qDebug()<<"Error: Discovery: bind failed "<<socket.errorString();
        return;
    }

    connect(&socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
}

void ReceiveThread::send(const char* buffer,int length) {
    if(socket.writeDatagram(buffer,length,QHostAddress(metisip),1024)<0) {
        qDebug()<<"Error: Discovery: writeDatagram failed "<<socket.errorString();
        return;
    }
}

void ReceiveThread::readyRead() {

    QHostAddress metisAddress;
    quint16 metisPort;
    unsigned char buffer[1024];

    qDebug()<<"Discovery::readyRead";

    if(socket.readDatagram((char*)&buffer,(qint64)sizeof(buffer),&metisAddress,&metisPort)>0) {

        if(buffer[0]==0xEF && buffer[1]==0xFE) {
            switch(buffer[2]) {
            case 3:  // erase completed
                qDebug()<<"commandCompleted";
                emit eraseCompleted();
                break;
            case 4:  // ready for next buffer
                qDebug()<<"ready for next buffer";
                emit nextBuffer();
                break;
            default:
                qDebug()<<"invalid reply="<<buffer[3];
                break;
            }
        } else {
            qDebug() << "received invalid response in ReceiveThread";
        }
    } else {
        qDebug()<<"ReceiveThread::readyRead: readDatagram failed";
    }
}

void ReceiveThread::stop() {
    socket.close();
}
