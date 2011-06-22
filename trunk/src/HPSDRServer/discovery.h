#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <QtNetwork/QUdpSocket>

#include "server.h"

class Discovery :public QObject {
    Q_OBJECT

public:
    Discovery(Server* s);

public slots:
    void readyRead();

private:
    Server* server;
};

#endif // DISCOVERY_H
