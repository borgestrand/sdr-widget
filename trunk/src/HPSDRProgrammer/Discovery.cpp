#include "QDebug"

#include "Discovery.h"
#include "Metis.h"


Discovery::Discovery(QString myip) {
    qDebug()<<"Discovery: "<<myip;
    ip=myip;

    if(!socket.bind(QHostAddress(ip),5001,QUdpSocket::ReuseAddressHint)) {
        qDebug()<<"Error: Discovery: bind failed "<<socket.errorString();
        return;
    }

    connect(&socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
}

void Discovery::discover() {
    // send the discovery packet
    unsigned char buffer[63];
    int i;

    buffer[0]=(char)0xEF; //header
    buffer[1]=(char)0XFE;
    buffer[2]=(char)0x02;
    for(i=3;i<63;i++) {
        buffer[i]=(char)0x00;
    }

    if(socket.writeDatagram((const char*)buffer,sizeof(buffer),QHostAddress::Broadcast,1024)<0) {
        qDebug()<<"Error: Discovery: writeDatagram failed "<<socket.errorString();
        return;
    }
}

void Discovery::stop() {
    socket.close();
}

void Discovery::readyRead() {

    QHostAddress metisAddress;
    quint16 metisPort;
    unsigned char buffer[1024];

    qDebug()<<"Discovery::readyRead";

    if(socket.readDatagram((char*)&buffer,(qint64)sizeof(buffer),&metisAddress,&metisPort)>0) {

        if(buffer[0]==0xEF && buffer[1]==0xFE) {
            switch(buffer[2]) {
            case 3:  // reply
                // should not happen on this port
                qDebug()<<"Discovery::readyRead: reply!!!";
                break;
            case 2:  // response to a discovery packet
                qDebug()<<"Discovery::readyRead: discovery response: "<<metisAddress.toString();
                if(metisAddress.toString()!=ip) {
                    Metis* metis=new Metis(metisAddress.toIPv4Address(),&buffer[3]);
                    emit metis_found(metis);
                } else {
                    qDebug()<<"Discovery::readyRead: from: "<<metisAddress.toString();
                }
                break;
            case 1:  // a data packet
                // should not happen on this port
                qDebug()<<"Discovery::readyRead: data!!!";
                break;
            }
        } else {
            qDebug() << "received invalid response to discovery";
        }
    } else {
        qDebug()<<"Discovery::readyRead: readDatagram failed";
    }
}
