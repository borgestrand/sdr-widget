#ifndef BANDSCOPE_H
#define BANDSCOPE_H

#include <QObject>
#include <QFrame>
#include <QtCore>
#include <QPainter>
#include <QPoint>
#include <QTimer>

#include "Connection.h"

#define BANDSCOPE_PORT 9000

namespace Ui {
    class Bandscope;
}

class Bandscope : public QFrame
{
    Q_OBJECT

public:
    explicit Bandscope(QWidget *parent = 0);
    ~Bandscope();

    void closeEvent(QCloseEvent* event);
    void connect(QString host);
    void disconnect();

public slots:
    void bandscopeBuffer(char* header,char* buffer);
    void connected();
    void disconnected(QString message);
    void updateBandscope();

protected:
    void paintEvent(QPaintEvent*);

private:
    Ui::Bandscope *ui;

    Connection connection;

    int bandscopeHigh;
    int bandscopeLow;
    QVector <QPoint> plot;
};

#endif // BANDSCOPE_H
