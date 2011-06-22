#include "audio.h"

Audio::Audio(Receiver* r) {
    receiver=r;
    socket.bind(AUDIO_PORT+(receiver->getRx()*2));
    connect(&socket,SIGNAL(readyRead()),this,SLOT(readyRead()));
}

void Audio::readyRead() {

    QHostAddress address;
    quint16 port;
    int bytes;

    if((bytes=socket.readDatagram((char*)&audio_buffer,(qint64)sizeof(audio_buffer),&address,&port))!=sizeof(audio_buffer)) {
        qDebug()<<"Error: Audio::readyRead: readDatagram failed "<<socket.errorString();
        return;
    }

    //qDebug()<<"Audio::readyRead "<<bytes;
    receiver->send_audio_buffer(audio_buffer);
}
