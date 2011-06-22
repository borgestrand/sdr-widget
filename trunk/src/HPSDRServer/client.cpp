#include <QDebug>

#include "client.h"

Client::Client(QTcpSocket* socket,Server* s) {
    qDebug()<<"new Client";
    server=s;
    client_address=socket->peerAddress();
    iq_port=-1;
    bandscope_port=-1;
    connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(discardClient()));
}

Client::~Client() {
    qDebug()<<"destroy Client";
}

int Client::get_iq_port() {
    return iq_port;
}

QHostAddress Client::get_client_address() {
    return client_address;
}

void Client::readClient() {
    //qDebug()<<"Client::readClient";
    QTcpSocket* socket = (QTcpSocket*)sender();
    QByteArray buffer=socket->readLine();
    QString response=parseCommand(QString(buffer));
    socket->write(response.toAscii());
}

QString Client::parseCommand(QString buffer) {
    QStringList token = buffer.split(QRegExp(" "));
    if(token[0]=="attach") {
        if(token.count()>=2) {
            rx=token[1];
            return server->attach(rx.toInt(),this);
        } else {
            return "Invalid attach command";
        }
    } else if(token[0]=="start") {
        if(token.count()>=3) {
            if(token[1]=="iq") {
                iq_port=token[2].toInt();
            } else if(token[1]=="bandscope") {
                bandscope_port=token[2].toInt();
            } else {
                return "Invalid start command";
            }
        } else {
            return "Invalid start command";
        }
    } else if(token[0]=="stop") {
        if(token.count()>=3) {
            if(token[1]=="iq") {
                iq_port=-1;
            } else if(token[1]=="bandscope") {
                bandscope_port=-1;
            } else {
                return "Invalid stop command";
            }
        } else {
            return "Invalid stop command";
        }
    } else if(token[0]=="frequency") {
        if(token.count()>=2) {
            frequency=token[1].toLong();
            server->setFrequency(rx.toInt(),frequency);
        } else {
            return "Invalid frequency command";
        }
    } else if(token[0]=="preamp") {
        if(token.count()>=2) {
            preamp=(token[1]=="on");
        } else {
            return "Invalid preamp command";
        }
    } else if(token[0]=="mox") {
        if(token.count()>=2) {
            mox=(token[1]=="1");
            server->setMox(mox);
            qDebug()<<"mox"<<token[1]<<mox;
        } else {
            return "Invalid mox command";
        }
    } else if(token[0]=="ocoutput") {
        if(token.count()>=2) {
            ocoutput=token[1].toInt();
        } else {
            return "Invalid ocoutput command";
        }
    } else if(token[0]=="record") {
        if(token.count()>=2) {
            record=(token[1]=="on");
        } else {
            return "Invalid record command";
        }
    }
    return "OK";
}

void Client::playAudio(float *buffer) {
    //qDebug()<<"Client::playAudio";
    server->playAudio(buffer);
}

void Client::discardClient() {
    qDebug()<<"Client::discardClient";
    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->close();

    // need to discard the receiver

}
