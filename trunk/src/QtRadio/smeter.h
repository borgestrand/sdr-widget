#ifndef SMETER_H
#define SMETER_H

#include <QtCore>

#include <QFrame>
#include <QPainter>
#include "Meter.h"


// class Spectrum: public QFrame {
//    Q_OBJECT
class sMeter: public QFrame {
    Q_OBJECT

public:
    sMeter();
    sMeter(QWidget*& widget);
    virtual ~sMeter();
    int meter_dbm;
    void setSubRxState(bool state);

protected:
    void paintEvent(QPaintEvent*);

signals:

public slots:

private:
    Meter* sMeterMain;
    Meter* sMeterSub;
    bool subRx;
};

#endif // SMETER_H
