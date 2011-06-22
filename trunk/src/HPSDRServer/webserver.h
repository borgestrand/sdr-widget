#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QTcpServer>

#include "server.h"

class WebServer : public QObject {
    Q_OBJECT

public:
    WebServer(int port);

public slots:
    void newConnection();

private:
    QTcpServer* webserver;
    QTcpSocket socket;
    Server server;

};

#endif
