#ifndef CLIENT_H
#define CLIENT_H


#include <QObject>
#include <QtNetwork>

#include "server.h"

class Client : public QObject {
    Q_OBJECT
public:
    Client(QTcpSocket* socket,Server* s);
    ~Client();

    int get_iq_port();

    QHostAddress get_client_address();

    void playAudio(float* buffer);

signals:

public slots:
    void readClient();
    void discardClient();

private:
    Server* server;
    QString parseCommand(QString buffer);

    QString rx;

    QHostAddress client_address;
    int iq_port;
    int bandscope_port;
    long frequency;

    int preamp;
    int mox;
    int ocoutput;
    int record;
};

#endif // CLIENT_H
