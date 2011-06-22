#include "webserver.h"
#include "webclient.h"

#include <QDebug>

WebServer::WebServer(int port) {
    //qDebug()<<"WebServer::WebServer";
    webserver=new QTcpServer();
    webserver->listen(QHostAddress::Any,port);
    connect(webserver,SIGNAL(newConnection()),this,SLOT(newConnection()));
    qDebug()<<"HPSDRServer web server listening on port "<<port;
}

void WebServer::newConnection() {
    //qDebug()<<"WebServer::newConnection";
    WebClient *client = new WebClient(webserver->nextPendingConnection(),&server);
}
