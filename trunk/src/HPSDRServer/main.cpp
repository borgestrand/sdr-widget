#include <QtCore/QCoreApplication>

#include <webserver.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    WebServer* webserver=new WebServer(8080);

    int rc=a.exec();

    qDebug()<<"a.exec returned "<<rc;
    return rc;
}
