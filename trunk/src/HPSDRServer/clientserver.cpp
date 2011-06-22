#include "client.h"

ClientServer::ClientServer(Server* s) {
    qDebug()<<"ClientServer::ClientServer listening on port 11000";
    server=s;
    clientserver=new QTcpServer();
    clientserver->listen(QHostAddress::Any,11000);
    connect(clientserver,SIGNAL(newConnection()),this,SLOT(newConnection()));
}

void ClientServer::newConnection() {
    qDebug()<<"ClientServer::newConnection";
    Client *client = new Client(clientserver->nextPendingConnection(),server);
}
