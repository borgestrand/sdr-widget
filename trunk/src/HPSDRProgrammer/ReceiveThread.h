#ifndef RECEIVETHREAD_H
#define RECEIVETHREAD_H

#include <sys/types.h>
#ifdef __WIN32
#include <winsock2.h>
#define socklen_t int
#else
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <ifaddrs.h>
#endif
#include <QThread>
#include <QUdpSocket>

class ReceiveThread : public QObject {
    Q_OBJECT
public:
    ReceiveThread(QString myip,QString metis);
    void send(const char* buffer,int length);
    void stop();
signals:
    void eraseCompleted();
    void nextBuffer();
    void timeout();
public slots:
    void readyRead();
private:
    unsigned char buffer[2048];

    QString myip;
    QString metisip;
    QUdpSocket socket;
};

#endif // RECEIVETHREAD_H
