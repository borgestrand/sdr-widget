#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <QThread>
#include <QUdpSocket>

#include "Metis.h"

class Discovery : public QObject {
    Q_OBJECT
public:
    Discovery(QString myip);
    void discover();
    void stop();
signals:
    void metis_found(Metis*);
    void reply(unsigned char);
public slots:
    void readyRead();
private:
    bool stopped;
    long ipAddress;
    int s;
    unsigned char buffer[2048];
    int bytes_read;

    QUdpSocket socket;
    QString ip;
    qint16 port;
};

#endif // DISCOVERY_H

