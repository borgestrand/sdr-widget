#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QUdpSocket>

#include "receiver.h"

#define AUDIO_PORT 15000

class Audio : public QObject {
    Q_OBJECT
public:
    Audio(Receiver* r);

signals:

public slots:
    void readyRead();

private:
    Receiver* receiver;

    QUdpSocket socket;

    float audio_buffer[BUFFER_SIZE*4];  // Audio left/right, Transmit I/Q

};

#endif // AUDIO_H
