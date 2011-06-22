#ifndef CLIENTSERVER_H
#define CLIENTSERVER_H

#include <QObject>
#include <QtNetwork>

class Server;
class Client;

class ClientServer : public QObject
{
    Q_OBJECT
public:
    ClientServer(Server* s);

signals:

public slots:
    void newConnection();

private:
    Server* server;
    QTcpServer* clientserver;

};

#endif // CLIENTSERVER_H
