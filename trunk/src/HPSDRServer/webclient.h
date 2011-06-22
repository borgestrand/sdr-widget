#ifndef WEBCLIENT_H
#define WEBCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QStringList>

#include "server.h"
#include "discovery.h"

class WebClient : public QObject {
    Q_OBJECT

public:
    WebClient(QTcpSocket* socket,Server* s);

public slots:
    void readClient();
    void discardClient();

private:
    Server* server;

    QString method;
    QString url;

    QMap<QString,QString> params;

    QTextStream* outputStream;

    void index();
    void device();
    void discover();
    void start();
    void stop();
    void configure();

    QUdpSocket socket;
};


#endif // WEBCLIENT_H
